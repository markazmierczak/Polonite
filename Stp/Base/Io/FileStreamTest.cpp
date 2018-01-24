// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/FileSystem/File.h"

#include "Base/Test/GTest.h"

namespace stp {

TEST(FileTest, ReadWrite) {
  TemporaryDirectory temp_dir;
  ASSERT_TRUE(temp_dir.Create());
  FilePath file_path = temp_dir.path().JoinAscii("read_write_file");
  File file(file_path,
            File::FLAG_CREATE | File::FLAG_READ |
                File::FLAG_WRITE);
  ASSERT_TRUE(file.IsValid());

  char data_to_write[] = "test";
  const int kTestDataSize = 4;

  // Write 0 bytes to the file.
  int bytes_written = file.Write(0, data_to_write, 0);
  EXPECT_EQ(0, bytes_written);

  // Write "test" to the file.
  bytes_written = file.Write(0, data_to_write, kTestDataSize);
  EXPECT_EQ(kTestDataSize, bytes_written);

  // Read from EOF.
  char data_read_1[32];
  int bytes_read = file.Read(kTestDataSize, data_read_1, kTestDataSize);
  EXPECT_EQ(0, bytes_read);

  // Read from somewhere in the middle of the file.
  const int kPartialReadOffset = 1;
  bytes_read = file.Read(kPartialReadOffset, data_read_1, kTestDataSize);
  EXPECT_EQ(kTestDataSize - kPartialReadOffset, bytes_read);
  for (int i = 0; i < bytes_read; i++)
    EXPECT_EQ(data_to_write[i + kPartialReadOffset], data_read_1[i]);

  // Read 0 bytes.
  bytes_read = file.Read(0, data_read_1, 0);
  EXPECT_EQ(0, bytes_read);

  // Read the entire file.
  bytes_read = file.Read(0, data_read_1, kTestDataSize);
  EXPECT_EQ(kTestDataSize, bytes_read);
  for (int i = 0; i < bytes_read; i++)
    EXPECT_EQ(data_to_write[i], data_read_1[i]);

  // Read again, but using the trivial native wrapper.
  bytes_read = file.ReadNoBestEffort(0, data_read_1, kTestDataSize);
  EXPECT_LE(bytes_read, kTestDataSize);
  for (int i = 0; i < bytes_read; i++)
    EXPECT_EQ(data_to_write[i], data_read_1[i]);

  // Write past the end of the file.
  const int kOffsetBeyondEndOfFile = 10;
  const int kPartialWriteLength = 2;
  bytes_written = file.Write(kOffsetBeyondEndOfFile,
                             data_to_write, kPartialWriteLength);
  EXPECT_EQ(kPartialWriteLength, bytes_written);

  // Make sure the file was extended.
  int64_t file_size = 0;
  EXPECT_TRUE(GetFileSize(file_path, &file_size));
  EXPECT_EQ(kOffsetBeyondEndOfFile + kPartialWriteLength, file_size);

  // Make sure the file was zero-padded.
  char data_read_2[32];
  bytes_read = file.Read(0, data_read_2, static_cast<int>(file_size));
  EXPECT_EQ(file_size, bytes_read);
  for (int i = 0; i < kTestDataSize; i++)
    EXPECT_EQ(data_to_write[i], data_read_2[i]);
  for (int i = kTestDataSize; i < kOffsetBeyondEndOfFile; i++)
    EXPECT_EQ(0, data_read_2[i]);
  for (int i = kOffsetBeyondEndOfFile; i < file_size; i++)
    EXPECT_EQ(data_to_write[i - kOffsetBeyondEndOfFile], data_read_2[i]);
}

TEST(FileTest, Append) {
  TemporaryDirectory temp_dir;
  ASSERT_TRUE(temp_dir.Create());
  FilePath file_path = temp_dir.path().JoinAscii("append_file");
  File file(file_path, File::FLAG_CREATE | File::FLAG_APPEND);
  ASSERT_TRUE(file.IsValid());

  char data_to_write[] = "test";
  const int kTestDataSize = 4;

  // Write 0 bytes to the file.
  int bytes_written = file.Write(0, data_to_write, 0);
  EXPECT_EQ(0, bytes_written);

  // Write "test" to the file.
  bytes_written = file.Write(0, data_to_write, kTestDataSize);
  EXPECT_EQ(kTestDataSize, bytes_written);

  file.Close();
  File file2(file_path,
             File::FLAG_OPEN | File::FLAG_READ |
                 File::FLAG_APPEND);
  ASSERT_TRUE(file2.IsValid());

  // Test passing the file around.
  file = std::move(file2);
  EXPECT_FALSE(file2.IsValid());
  ASSERT_TRUE(file.IsValid());

  char append_data_to_write[] = "78";
  const int kAppendDataSize = 2;

  // Append "78" to the file.
  bytes_written = file.Write(0, append_data_to_write, kAppendDataSize);
  EXPECT_EQ(kAppendDataSize, bytes_written);

  // Read the entire file.
  char data_read_1[32];
  int bytes_read = file.Read(0, data_read_1,
                             kTestDataSize + kAppendDataSize);
  EXPECT_EQ(kTestDataSize + kAppendDataSize, bytes_read);
  for (int i = 0; i < kTestDataSize; i++)
    EXPECT_EQ(data_to_write[i], data_read_1[i]);
  for (int i = 0; i < kAppendDataSize; i++)
    EXPECT_EQ(append_data_to_write[i], data_read_1[kTestDataSize + i]);
}


TEST(FileTest, Length) {
  TemporaryDirectory temp_dir;
  ASSERT_TRUE(temp_dir.Create());
  FilePath file_path = temp_dir.path().JoinAscii("truncate_file");
  File file(file_path,
            File::FLAG_CREATE | File::FLAG_READ |
                File::FLAG_WRITE);
  ASSERT_TRUE(file.IsValid());
  EXPECT_EQ(0, file.GetLength());

  // Write "test" to the file.
  char data_to_write[] = "test";
  int kTestDataSize = 4;
  int bytes_written = file.Write(0, data_to_write, kTestDataSize);
  EXPECT_EQ(kTestDataSize, bytes_written);

  // Extend the file.
  const int kExtendedFileLength = 10;
  int64_t file_size = 0;
  EXPECT_TRUE(file.SetLength(kExtendedFileLength));
  EXPECT_EQ(kExtendedFileLength, file.GetLength());
  EXPECT_TRUE(GetFileSize(file_path, &file_size));
  EXPECT_EQ(kExtendedFileLength, file_size);

  // Make sure the file was zero-padded.
  char data_read[32];
  int bytes_read = file.Read(0, data_read, static_cast<int>(file_size));
  EXPECT_EQ(file_size, bytes_read);
  for (int i = 0; i < kTestDataSize; i++)
    EXPECT_EQ(data_to_write[i], data_read[i]);
  for (int i = kTestDataSize; i < file_size; i++)
    EXPECT_EQ(0, data_read[i]);

  // Truncate the file.
  const int kTruncatedFileLength = 2;
  EXPECT_TRUE(file.SetLength(kTruncatedFileLength));
  EXPECT_EQ(kTruncatedFileLength, file.GetLength());
  EXPECT_TRUE(GetFileSize(file_path, &file_size));
  EXPECT_EQ(kTruncatedFileLength, file_size);

  // Make sure the file was truncated.
  bytes_read = file.Read(0, data_read, kTestDataSize);
  EXPECT_EQ(file_size, bytes_read);
  for (int i = 0; i < file_size; i++)
    EXPECT_EQ(data_to_write[i], data_read[i]);
}

TEST(FileTest, ReadAtCurrentPosition) {
  TemporaryDirectory temp_dir;
  ASSERT_TRUE(temp_dir.Create());
  FilePath file_path = temp_dir.path().JoinAscii("read_at_current_position");
  File file(file_path,
            File::FLAG_CREATE | File::FLAG_READ |
                File::FLAG_WRITE);
  EXPECT_TRUE(file.IsValid());

  const char kData[] = "test";
  const int kDataSize = sizeof(kData) - 1;
  EXPECT_EQ(kDataSize, file.Write(0, kData, kDataSize));

  EXPECT_EQ(0, file.Seek(File::FromBegin, 0));

  char buffer[kDataSize];
  int first_chunk_size = kDataSize / 2;
  EXPECT_EQ(first_chunk_size, file.ReadAtCurrentPos(buffer, first_chunk_size));
  EXPECT_EQ(kDataSize - first_chunk_size,
            file.ReadAtCurrentPos(buffer + first_chunk_size,
                                  kDataSize - first_chunk_size));
  EXPECT_EQ(std::string(buffer, buffer + kDataSize), std::string(kData));
}

TEST(FileTest, WriteAtCurrentPosition) {
  TemporaryDirectory temp_dir;
  ASSERT_TRUE(temp_dir.Create());
  FilePath file_path = temp_dir.path().JoinAscii("write_at_current_position");
  File file(file_path,
            File::FLAG_CREATE | File::FLAG_READ |
                File::FLAG_WRITE);
  EXPECT_TRUE(file.IsValid());

  const char kData[] = "test";
  const int kDataSize = sizeof(kData) - 1;

  int first_chunk_size = kDataSize / 2;
  EXPECT_EQ(first_chunk_size, file.WriteAtCurrentPos(kData, first_chunk_size));
  EXPECT_EQ(kDataSize - first_chunk_size,
            file.WriteAtCurrentPos(kData + first_chunk_size,
                                   kDataSize - first_chunk_size));

  char buffer[kDataSize];
  EXPECT_EQ(kDataSize, file.Read(0, buffer, kDataSize));
  EXPECT_EQ(std::string(buffer, buffer + kDataSize), std::string(kData));
}

TEST(FileTest, Seek) {
  TemporaryDirectory temp_dir;
  ASSERT_TRUE(temp_dir.Create());
  FilePath file_path = temp_dir.path().JoinAscii("seek_file");
  File file(file_path,
            File::FLAG_CREATE | File::FLAG_READ |
                File::FLAG_WRITE);
  ASSERT_TRUE(file.IsValid());

  const int64_t kOffset = 10;
  EXPECT_EQ(kOffset, file.Seek(File::FromBegin, kOffset));
  EXPECT_EQ(2 * kOffset, file.Seek(File::FromCurrent, kOffset));
  EXPECT_EQ(kOffset, file.Seek(File::FromCurrent, -kOffset));
  EXPECT_TRUE(file.SetLength(kOffset * 2));
  EXPECT_EQ(kOffset, file.Seek(File::FromEnd, -kOffset));
}

} // namespace stp
