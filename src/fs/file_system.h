#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "file.h"
#include <optional>
#include <list>

class FileSystem
{
public:
    FileSystem(std::string root_dir);
    virtual ~FileSystem() = default;

    bool                  isExist(std::string const & fname) const;
    std::optional<InFile> getFile(std::string const & fname) const;   // ex. file name: "fonts/times.ttf"
    size_t                getNumFiles() const { return m_files.size(); }

    bool writeFile(BaseFile const & file, std::string path = {});   // Memory file
    bool createZIP(std::vector<BaseFile const *> filelist,
                   std::string const &           zipname);   // all zip files saves in root directory
    bool addFileToZIP(BaseFile const * file, std::string const & zipname);

    static std::string GetTempDir();
    static std::string GetCurrentDir();
    static std::string GetTempFileName();

private:
    struct FileData
    {
        struct ZFileData
        {
            bool        compressed        = false;
            size_t      compressed_size   = 0;
            size_t      uncompressed_size = 0;
            size_t      lfh_offset        = 0;
            std::string fname;
        };

        bool        is_zip = false;
        ZFileData   zip_data;
        std::string fname;
    };

    void   addZippedDir(std::string const & fname);
    InFile loadRegularFile(FileData const & f) const;
    InFile loadZipFile(FileData const & zf) const;

    std::list<FileData> m_files;
    std::string         m_data_dir;
};

#endif   // FILESYSTEM_H
