#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "file.h"
#include <optional>
#include <list>

namespace evnt
{
class FileSystem
{
public:
    FileSystem(std::string root_dir);
    virtual ~FileSystem() = default;

    bool                  isExist(std::string const & fname) const;
    std::optional<InFile> getFile(std::string const & fname) const;   // ex. file name: "fonts/times.ttf"
    size_t                getNumFiles() const { return m_files.size(); }

    bool writeFile(std::string const & path, BaseFile const * file);   // Memory file
    bool createZIP(std::vector<BaseFile const *> filelist,
                   std::string const &           zipname);   // all zip files saves in root directory
    bool addFileToZIP(BaseFile const * file, std::string const & zipname);

    static std::string GetTempDir();
    static std::string GetCurrentDir();
    static std::string GetTempFileName();

private:
    struct file_data
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
    InFile loadRegularFile(file_data const & f) const;
    InFile loadZipFile(file_data const & zf) const;

    std::list<file_data> m_files;
    std::string          m_data_dir;
};
}   // namespace evnt

#endif   // FILESYSTEM_H
