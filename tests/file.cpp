#include <tmp/directory>
#include <tmp/file>

#include <gtest/gtest.h>

namespace tmp {

namespace fs = std::filesystem;

/// Tests file creation with prefix
TEST(file, create_with_prefix) {
    file tmpfile = file(PREFIX);
    fs::path parent = tmpfile->parent_path();

    EXPECT_TRUE(fs::exists(tmpfile));
    EXPECT_TRUE(fs::is_regular_file(tmpfile));
    EXPECT_TRUE(fs::equivalent(parent, fs::temp_directory_path() / PREFIX));
}

/// Tests file creation without prefix
TEST(file, create_without_prefix) {
    file tmpfile = file();
    fs::path parent = tmpfile->parent_path();

    EXPECT_TRUE(fs::exists(tmpfile));
    EXPECT_TRUE(fs::is_regular_file(tmpfile));
    EXPECT_TRUE(fs::equivalent(parent, fs::temp_directory_path()));
}

/// Tests file creation with suffix
TEST(file, create_with_suffix) {
    file tmpfile = file("", ".test");

    EXPECT_TRUE(fs::exists(tmpfile));
    EXPECT_TRUE(fs::is_regular_file(tmpfile));
    EXPECT_EQ(tmpfile->extension(), ".test");
}

/// Tests multiple file creation with the same prefix
TEST(file, create_multiple) {
    file fst = file(PREFIX);
    file snd = file(PREFIX);

    EXPECT_FALSE(fs::equivalent(fst, snd));
}

/// Tests creation of a temporary copy of a file
TEST(file, copy_file) {
    file tmpfile = file(PREFIX);
    tmpfile.write("Hello, world!");

    file tmpcopy = file::copy(tmpfile, PREFIX);
    EXPECT_TRUE(fs::exists(tmpfile));
    EXPECT_TRUE(fs::exists(tmpcopy));
    EXPECT_FALSE(fs::equivalent(tmpfile, tmpcopy));

    EXPECT_TRUE(fs::is_regular_file(tmpfile));

    EXPECT_EQ(tmpcopy.read(), "Hello, world!");
}

/// Tests creation of a temporary copy of a directory
TEST(file, copy_directory) {
    directory tmpdir = directory(PREFIX);
    EXPECT_THROW(file::copy(tmpdir, PREFIX), fs::filesystem_error);
}

/// Tests binary file reading
TEST(file, read_binary) {
    file tmpfile = file(PREFIX);
    std::ofstream stream = std::ofstream(tmpfile.path(), std::ios::binary);

    stream << "Hello," << std::endl;
    stream << "world!" << std::endl;

    EXPECT_EQ(tmpfile.read(), "Hello,\nworld!\n");
}

/// Tests text file reading
TEST(file, read_text) {
    file tmpfile = file::text(PREFIX);
    std::ofstream stream = std::ofstream(tmpfile.path());

    stream << "Hello," << std::endl;
    stream << "world!" << std::endl;

    EXPECT_EQ(tmpfile.read(), "Hello,\nworld!\n");
}

/// Tests binary file writing
TEST(file, write_binary) {
    file tmpfile = file(PREFIX);
    tmpfile.write("Hello\n");

    {
        auto stream = std::ifstream(tmpfile.path(), std::ios::binary);
        auto content = std::string(std::istreambuf_iterator<char>(stream), {});
        EXPECT_EQ(content, "Hello\n");
    }

    tmpfile.write("world!\n");

    {
        auto stream = std::ifstream(tmpfile.path(), std::ios::binary);
        auto content = std::string(std::istreambuf_iterator<char>(stream), {});
        EXPECT_EQ(content, "world!\n");
    }
}

/// Tests text file writing
TEST(file, write_text) {
    file tmpfile = file::text(PREFIX);
    tmpfile.write("Hello\n");

    {
        auto stream = std::ifstream(tmpfile.path());
        auto content = std::string(std::istreambuf_iterator<char>(stream), {});
        EXPECT_EQ(content, "Hello\n");
    }

    tmpfile.write("world!\n");

    {
        auto stream = std::ifstream(tmpfile.path());
        auto content = std::string(std::istreambuf_iterator<char>(stream), {});
        EXPECT_EQ(content, "world!\n");
    }
}

/// Tests binary file appending
TEST(file, append_binary) {
    file tmpfile = file(PREFIX);
    std::ofstream(tmpfile.path(), std::ios::binary) << "Hello, ";

    tmpfile.append("world");

    {
        auto stream = std::ifstream(tmpfile.path(), std::ios::binary);
        auto content = std::string(std::istreambuf_iterator<char>(stream), {});
        EXPECT_EQ(content, "Hello, world");
    }

    tmpfile.append("!");

    {
        auto stream = std::ifstream(tmpfile.path(), std::ios::binary);
        auto content = std::string(std::istreambuf_iterator<char>(stream), {});
        EXPECT_EQ(content, "Hello, world!");
    }
}

/// Tests text file appending
TEST(file, append_text) {
    file tmpfile = file::text(PREFIX);
    std::ofstream(tmpfile.path()) << "Hello,\n ";

    tmpfile.append("world");

    {
        auto stream = std::ifstream(tmpfile.path());
        auto content = std::string(std::istreambuf_iterator<char>(stream), {});
        EXPECT_EQ(content, "Hello,\n world");
    }

    tmpfile.append("!");

    {
        auto stream = std::ifstream(tmpfile.path());
        auto content = std::string(std::istreambuf_iterator<char>(stream), {});
        EXPECT_EQ(content, "Hello,\n world!");
    }
}

/// Tests that destructor removes a file
TEST(file, destructor) {
    fs::path path = fs::path();
    {
        file tmpfile = file(PREFIX);
        path = tmpfile;
    }

    EXPECT_FALSE(fs::exists(path));
}

/// Tests file move constructor
TEST(file, move_constructor) {
    file fst = file(PREFIX);
    file snd = std::move(fst);

    EXPECT_TRUE(fst.path().empty());
    EXPECT_TRUE(fs::exists(snd));
}

/// Tests file move assignment operator
TEST(file, move_assignment) {
    file fst = file(PREFIX);
    file snd = file(PREFIX);

    fs::path path1 = fst;
    fs::path path2 = snd;

    fst = std::move(snd);

    EXPECT_FALSE(fs::exists(path1));
    EXPECT_TRUE(fs::exists(path2));

    EXPECT_TRUE(fs::exists(fst));
    EXPECT_TRUE(fs::equivalent(fst, path2));
}

/// Tests file releasing
TEST(file, release) {
    fs::path path = fs::path();
    {
        file tmpfile = file(PREFIX);
        fs::path expected = tmpfile;
        path = tmpfile.release();

        EXPECT_TRUE(fs::equivalent(path, expected));
    }

    EXPECT_TRUE(fs::exists(path));
    fs::remove(path);
}

/// Tests file moving
TEST(file, move) {
    fs::path path = fs::path();
    fs::path to = fs::temp_directory_path() / PREFIX / "non-existing/parent";
    {
        file tmpfile = file(PREFIX);
        path = tmpfile;

        tmpfile.move(to);
    }

    EXPECT_FALSE(fs::exists(path));
    EXPECT_TRUE(fs::exists(to));
    fs::remove_all(fs::temp_directory_path() / PREFIX / "non-existing");
}
}    // namespace tmp
