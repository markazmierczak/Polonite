// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Debug/StackTrace.h"

#include "Base/Debug/Assert.h"
#include "Base/Debug/Log.h"
#include "Base/Io/FileStream.h"
#include "Base/Io/TextWriter.h"
#include "Base/Posix/EintrWrapper.h"
#include "Base/Text/FormatMany.h"

#include <elf.h>
#include <fcntl.h>
#include <unistd.h>

#if !OS(ANDROID)
#include <link.h>  // For ElfW() macro.
#endif

namespace stp {

/*
// Returns elf_header.e_type if the file pointed by fd is an ELF binary.
static int FileGetElfType(FileStream& file) {
  ElfW(Ehdr) elf_header;

  int count = sizeof(elf_header);
  file.PositionalRead(0, (byte_t*)&elf_header, count);

  if (memcmp(elf_header.e_ident, ELFMAG, SELFMAG) != 0)
    throw Exception() << "not ELF file";

  return elf_header.e_type;
}

// Read the section headers in the given ELF binary, and if a section
// of the specified type is found, set the output to this section header
// and return true.  Otherwise, return false.
// To keep stack consumption low, we would like this function to not get inlined.
static NEVER_INLINE bool
GetSectionHeaderByType(
    FileStream& file,
    ElfW(Half) sh_num, const off_t sh_offset,
    ElfW(Word) type, ElfW(Shdr) *out) {
  // Read at most 16 section headers at a time to save read calls.
  ElfW(Shdr) buf[16];
  for (int i = 0; i < sh_num;) {
    int num_bytes_left = (sh_num - i) * sizeof(buf[0]);
    int num_bytes_to_read = (SizeOf(buf) > num_bytes_left) ? num_bytes_left : SizeOf(buf);
    int len = file.PositionalRead(
        sh_offset + i * sizeof(buf[0]), (byte_t*)buf, num_bytes_to_read);
    ASSERT(len % sizeof(buf[0]) == 0);
    int num_headers_in_buf = len / sizeof(buf[0]);
    ASSERT(num_headers_in_buf <= isizeofArray(buf));
    for (int j = 0; j < num_headers_in_buf; ++j) {
      if (buf[j].sh_type == type) {
        *out = buf[j];
        return true;
      }
    }
    i += num_headers_in_buf;
  }
  return false;
}

// There is no particular reason to limit section name to 63 characters,
// but there has (as yet) been no need for anything longer either.
const int MaxSectionNameLen = 64;

// name_len should include terminating '\0'.
bool GetSectionHeaderByName(
    FileStream& file,
    const char* name, int name_len,
    ElfW(Shdr) *out) {
  ElfW(Ehdr) elf_header;
  if (!file.ReadAtExact(0, (byte_t*)&elf_header, sizeof(elf_header)))
    return false;

  ElfW(Shdr) shstrtab;
  int shstrtab_offset = (elf_header.e_shoff + elf_header.e_shentsize * elf_header.e_shstrndx);
  if (!file.ReadAtExact(shstrtab_offset, (byte_t*)&shstrtab, sizeof(shstrtab)))
    return false;

  for (int i = 0; i < elf_header.e_shnum; ++i) {
    int section_header_offset = (elf_header.e_shoff + elf_header.e_shentsize * i);
    if (!file.ReadAtExact(section_header_offset, (byte_t*)out, sizeof(*out)))
      return false;

    char header_name[MaxSectionNameLen];
    if (SizeOf(header_name) < name_len) {
      LOG(WARN, "section name is too long");
      // No point in even trying.
      return false;
    }
    int name_offset = shstrtab.sh_offset + out->sh_name;
    int n_read = file.PositionalRead(name_offset, (byte_t*)&header_name, name_len);
    if (n_read == -1)
      return false;
    if (n_read != name_len) {
      // Short read -- name could be at end of file.
      continue;
    }
    if (memcmp(header_name, name, name_len) == 0)
      return true;
  }
  return false;
}

// Read a symbol table and look for the symbol containing the
// pc. Iterate over symbols in a symbol table and look for the symbol
// containing "pc".  On success, return true and write the symbol name
// to out.  Otherwise, return false.
// To keep stack consumption low, we would like this function to not get inlined.
static NEVER_INLINE bool
FindSymbol(uint64_t pc,
           FileStream& file,
           byte_t* out, int out_size,
           uint64_t symbol_offset, const ElfW(Shdr) *strtab,
           const ElfW(Shdr) *symtab) {
  if (!symtab)
    return false;

  int num_symbols = symtab->sh_size / symtab->sh_entsize;
  for (int i = 0; i < num_symbols;) {
    int offset = symtab->sh_offset + i * symtab->sh_entsize;

    // If we are reading Elf64_Sym's, we want to limit this array to
    // 32 elements (to keep stack consumption low), otherwise we can
    // have a 64 element Elf32_Sym array.
    #if __WORDSIZE == 64
    #define NUM_SYMBOLS 32
    #else
    #define NUM_SYMBOLS 64
    #endif

    // Read at most NUM_SYMBOLS symbols at once to save read() calls.
    ElfW(Sym) buf[NUM_SYMBOLS];
    int len = file.PositionalRead(offset, (byte_t*)&buf, sizeof(buf));
    ASSERT(len % SizeOf(buf[0]) == 0);
    int num_symbols_in_buf = len / SizeOf(buf[0]);
    ASSERT(num_symbols_in_buf <= isizeofArray(buf));
    for (int j = 0; j < num_symbols_in_buf; ++j) {
      const ElfW(Sym)& symbol = buf[j];
      uint64_t start_address = symbol.st_value;
      start_address += symbol_offset;
      uint64_t end_address = start_address + symbol.st_size;
      if (symbol.st_value != 0 &&  // Skip null value symbols.
          symbol.st_shndx != 0 &&  // Skip undefined symbols.
          start_address <= pc && pc < end_address) {
        int len1 = file.PositionalRead(strtab->sh_offset + symbol.st_name,
                               out, out_size);
        return len1 > 0 && memchr(out, '\0', out_size) != nullptr;
      }
    }
    i += num_symbols_in_buf;
  }
  return false;
}

// Get the symbol name of "pc" from the file pointed by "fd".  Process
// both regular and dynamic symbol tables if necessary.  On success,
// write the symbol name to "out" and return true.  Otherwise, return false.
static bool GetSymbolFromObjectFile(
    FileStream& file,
    uint64_t pc,
    byte_t* out, int out_size,
    uint64_t map_base_address) {
  // Read the ELF header.
  ElfW(Ehdr) elf_header;
  if (!file.ReadAtExact(0, (byte_t*)&elf_header, sizeof(elf_header)))
    return false;

  uint64_t symbol_offset = 0;
  if (elf_header.e_type == ET_DYN) {  // DSO needs offset adjustment.
    ElfW(Phdr) phdr;
    // We need to find the PT_LOAD segment corresponding to the read-execute
    // file mapping in order to correctly perform the offset adjustment.
    for (unsigned i = 0; i != elf_header.e_phnum; ++i) {
      if (!file.ReadAtExact(elf_header.e_phoff + i * sizeof(phdr),
                            (byte_t*)&phdr, sizeof(phdr))) {
        return false;
      }
      if (phdr.p_type == PT_LOAD &&
          (phdr.p_flags & (PF_R | PF_X)) == (PF_R | PF_X)) {
        // Find the mapped address corresponding to virtual address zero. We do
        // this by first adding p_offset. This gives us the mapped address of
        // the start of the segment, or in other words the mapped address
        // corresponding to the virtual address of the segment. (Note that this
        // is distinct from the start address, as p_offset is not guaranteed to
        // be page aligned.) We then subtract p_vaddr, which takes us to virtual
        // address zero.
        symbol_offset = map_base_address + phdr.p_offset - phdr.p_vaddr;
        break;
      }
    }
    if (symbol_offset == 0)
      return false;
  }

  ElfW(Shdr) symtab, strtab;

  // Consult a regular symbol table first.
  if (GetSectionHeaderByType(file, elf_header.e_shnum, elf_header.e_shoff,
                             SHT_SYMTAB, &symtab)) {
    if (!file.ReadAtExact(elf_header.e_shoff + symtab.sh_link * sizeof(symtab),
                          (byte_t*)&strtab, sizeof(strtab))) {
      return false;
    }
    if (FindSymbol(pc, file, out, out_size, symbol_offset,
                   &strtab, &symtab)) {
      return true;  // Found the symbol in a regular symbol table.
    }
  }

  // If the symbol is not found, then consult a dynamic symbol table.
  if (GetSectionHeaderByType(file, elf_header.e_shnum, elf_header.e_shoff,
                             SHT_DYNSYM, &symtab)) {
    if (!file.ReadAtExact(elf_header.e_shoff + symtab.sh_link * sizeof(symtab),
                          (byte_t*)&strtab, sizeof(strtab))) {
      return false;
    }
    if (FindSymbol(pc, file, out, out_size, symbol_offset,
                   &strtab, &symtab)) {
      return true;  // Found the symbol in a dynamic symbol table.
    }
  }

  return false;
}

namespace {

// Helper class for reading lines from file.
//
// Note: we don't use ProcMapsIterator since the object is big (it has
// a 5k array member) and uses async-unsafe functions such as sscanf()
// and snprintf().
class LineReader {
 public:
  explicit LineReader(FileStream& file, byte_t* buf, int buf_len)
      : file_(file),
        buf_(buf), buf_len_(buf_len),
        bol_(buf), eol_(buf), eod_(buf) {}

  // Read '\n'-terminated line from file.  On success, modify "bol"
  // and "eol", then return true.  Otherwise, return false.
  //
  // Note: if the last line doesn't end with '\n', the line will be
  // dropped.  It's an intentional behavior to make the code simple.
  bool ReadLine(const char*& bol, const char*& eol) {
    if (BufferIsEmpty()) {  // First time.
      const int num_bytes = file_.ReadAtMost(buf_, buf_len_);
      if (num_bytes <= 0) {  // EOF or error.
        return false;
      }
      eod_ = buf_ + num_bytes;
      bol_ = buf_;
    } else {
      bol_ = eol_ + 1;  // Advance to the next line in the buffer.
      ASSERT(bol_ <= eod_);  // "bol_" can point to "eod_".
      if (!HasCompleteLine()) {
        int incomplete_line_length = eod_ - bol_;
        // Move the trailing incomplete line to the beginning.
        Arraymove(buf_, bol_, incomplete_line_length);
        // Read text from file and append it.
        byte_t* const append_pos = buf_ + incomplete_line_length;
        int capacity_left = buf_len_ - incomplete_line_length;

        int num_bytes = file_.ReadAtMost(append_pos, capacity_left);
        if (num_bytes <= 0) // EOF or error.
          return false;

        eod_ = append_pos + num_bytes;
        bol_ = buf_;
      }
    }
    eol_ = FindLineFeed();
    if (!eol_)  // '\n' not found.  Malformed line.
      return false;

    *eol_ = '\0';  // Replace '\n' with '\0'.

    bol = reinterpret_cast<char*>(bol_);
    eol = reinterpret_cast<char*>(eol_);
    return true;
  }

 private:
  explicit LineReader(const LineReader&);
  void operator=(const LineReader&);

  byte_t* FindLineFeed() {
    return reinterpret_cast<byte_t*>(memchr(bol_, '\n', eod_ - bol_));
  }

  bool BufferIsEmpty() { return buf_ == eod_; }

  bool HasCompleteLine() {
    return !BufferIsEmpty() && FindLineFeed() != NULL;
  }

  FileStream& file_;
  byte_t* const buf_;
  const int buf_len_;
  byte_t* bol_;
  byte_t* eol_;
  const byte_t* eod_;  // End of data in "buf_".
};
} // namespace

// Place the hex number read from "start" into "*hex".  The pointer to
// the first non-hex character or "end" is returned.
static char* GetHex(const char* start, const char* end, uint64_t *hex) {
  *hex = 0;
  const char* p;
  for (p = start; p < end; ++p) {
    int ch = *p;
    if ((ch >= '0' && ch <= '9') ||
        (ch >= 'A' && ch <= 'F') || (ch >= 'a' && ch <= 'f')) {
      *hex = (*hex << 4) | (ch < 'A' ? ch - '0' : (ch & 0xF) + 9);
    } else {  // Encountered the first non-hex character.
      break;
    }
  }
  ASSERT(p <= end);
  return const_cast<char* >(p);
}

// Searches for the object file (from /proc/self/maps) that contains
// the specified pc.  If found, sets |start_address| to the start address
// of where this object file is mapped in memory, sets the module base
// address into |base_address|, copies the object file name into
// |out_file_name|, and attempts to open the object file.  If the object
// file is opened successfully, returns the file descriptor.  Otherwise,
// returns -1.  |out_file_name_size| is the size of the file name buffer
// (including the null-terminator).
static NEVER_INLINE int OpenObjectFileContainingPc(
    uint64_t pc, uint64_t& base_address,
    char* out_file_name, int out_file_name_size) {
  FileStream maps_file;
  if (!maps_file.TryOpen(
      FILE_PATH_LITERAL("/proc/self/maps"),
      FileMode::OpenExisting, FileAccess::ReadOnly)) {
    return -1;
  }

  uint64_t start_address = 0;

  // Iterate over maps and look for the map containing the pc.  Then
  // look into the symbol tables inside.
  byte_t buf[1024];  // Big enough for line of sane /proc/self/maps
  int num_maps = 0;
  LineReader reader(maps_file, buf, sizeof(buf));
  while (true) {
    num_maps++;
    const char* cursor;
    const char* eol;
    if (!reader.ReadLine(cursor, eol))  // EOF or malformed line.
      return -1;

    // Start parsing line in /proc/self/maps.  Here is an example:
    //
    // 08048000-0804c000 r-xp 00000000 08:01 2142121    /bin/cat
    //
    // We want start address (08048000), end address (0804c000), flags
    // (r-xp) and file name (/bin/cat).

    // Read start address.
    cursor = GetHex(cursor, eol, &start_address);
    if (cursor == eol || *cursor != '-') {
      return -1;  // Malformed line.
    }
    ++cursor;  // Skip '-'.

    // Read end address.
    uint64_t end_address;
    cursor = GetHex(cursor, eol, &end_address);
    if (cursor == eol || *cursor != ' ') {
      return -1;  // Malformed line.
    }
    ++cursor;  // Skip ' '.

    // Check start and end addresses.
    if (!(start_address <= pc && pc < end_address)) {
      continue;  // We skip this map.  PC isn't in this map.
    }

    // Read flags.  Skip flags until we encounter a space or eol.
    const char* const flags_start = cursor;
    while (cursor < eol && *cursor != ' ') {
      ++cursor;
    }
    // We expect at least four letters for flags (ex. "r-xp").
    if (cursor == eol || cursor < flags_start + 4) {
      return -1;  // Malformed line.
    }

    // Check flags.  We are only interested in "r-x" maps.
    if (memcmp(flags_start, "r-x", 3) != 0) {  // Not a "r-x" map.
      continue;  // We skip this map.
    }
    ++cursor;  // Skip ' '.

    // Read file offset.
    uint64_t file_offset;
    cursor = GetHex(cursor, eol, &file_offset);
    if (cursor == eol || *cursor != ' ') {
      return -1;  // Malformed line.
    }
    ++cursor;  // Skip ' '.

    // Don't subtract 'start_address' from the first entry:
    // * If a binary is compiled w/o -pie, then the first entry in
    //   process maps is likely the binary itself (all dynamic libs
    //   are mapped higher in address space). For such a binary,
    //   instruction offset in binary coincides with the actual
    //   instruction address in virtual memory (as code section
    //   is mapped to a fixed memory range).
    // * If a binary is compiled with -pie, all the modules are
    //   mapped high at address space (in particular, higher than
    //   shadow memory of the tool), so the module can't be the
    //   first entry.
    base_address = ((num_maps == 1) ? 0U : start_address) - file_offset;

    // Skip to file name.  "cursor" now points to dev.  We need to
    // skip at least two spaces for dev and inode.
    int num_spaces = 0;
    while (cursor < eol) {
      if (*cursor == ' ') {
        ++num_spaces;
      } else if (num_spaces >= 2) {
        // The first non-space character after skipping two spaces
        // is the beginning of the file name.
        break;
      }
      ++cursor;
    }
    if (cursor == eol) {
      return -1;  // Malformed line.
    }

    // Finally, "cursor" now points to file name of our interest.
    int object_fd = HANDLE_EINTR(open(cursor, O_RDONLY));
    if (object_fd < 0) {
      // Failed to open object file.  Copy the object file name to
      // |out_file_name|.
      strncpy(out_file_name, cursor, out_file_name_size);
      // Making sure |out_file_name| is always null-terminated.
      out_file_name[out_file_name_size - 1] = '\0';
      return -1;
    }
    return object_fd;
  }
}

static NEVER_INLINE void FormatSymbolImpl(TextWriter& out, void* pc) {
  constexpr int MangledSize = 256;
  char mangled[MangledSize];

  constexpr int FilenameSize = 256;
  char filename_cstr[FilenameSize];
  filename_cstr[0] = '\0';

  uint64_t pc0 = reinterpret_cast<uintptr_t>(pc);

  uint64_t base_address = 0;
  int object_fd = OpenObjectFileContainingPc(pc0, base_address, filename_cstr, FilenameSize);
  if (object_fd == InvalidNativeFile)
    return false;

  FileStream object_file;
  object_file.OpenNative(object_fd);

  int elf_type = FileGetElfType(object_file);

  GetSymbolFromObjectFile(
      object_file, pc0,
      reinterpret_cast<byte_t*>(mangled), MangledSize,
      base_address);

  DemangleSymbols(out, makeSpanFromNullTerminated(mangled));

  auto filename = makeSpanFromNullTerminated(filename_cstr);
  if (!filename.IsEmpty()) {
    out << " at " << filename;
  }
}*/

void FormatSymbol(TextWriter& out, void *pc) {
//  try {
//    FormatSymbolImpl(out, pc);
//  } catch (Exception& exception) {
      out << "symbol ";
//    format(out, pc);
//  }
}

} // namespace stp
