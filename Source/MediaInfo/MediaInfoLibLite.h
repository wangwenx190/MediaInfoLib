#pragma once

#include "MediaInfoLibLite_global.h"
#include "File__Analyse_Automatic.h"
#include <string>
#include <string_view>
#include <memory>

namespace MediaInfoLibLite {

#if defined(UNICODE) || defined(_UNICODE)
    using str_t = std::wstring;
#else
    using str_t = std::string;
#endif

    enum struct StreamType : std::uint8_t {
        General,
        Video,
        Audio,
        Subtitle,
        Chapter
    };

    enum struct InfoType : bool {
        Value,
        Title
    };

    class ParserPrivate;
    class MEDIAINFOLIBLITE_API Parser final {
    public:
        Parser();
        Parser(str_t path);
        ~Parser();

        [[nodiscard]] const str_t& filePath() const;

        // The file will be parsed immediately if the given path is valid and accessible.
        // If the given path is the same with the previous one and it has been parsed successfully, nothing will happen.
        // If the given path is the same with the previous one and we fail to parse it previously, we'll try to parse it again.
        // If the given path is different with the previous one, all cached data will be cleared and we'll try to parse the new file.
        void setFilePath(str_t path);

        [[nodiscard]] bool isValid() const;
        [[nodiscard]] inline explicit operator bool() const { return isValid(); }

        // Release all internal resources.
        void reset();

        // The query result will be cached automatically, so calling this function repeatedly won't have performance impact.
        // This query operation will happen lazily, so it won't cost you anything if this function is never called.
        [[nodiscard]] const str_t& inform() const;

        // The query result will be cached automatically, so calling this function repeatedly won't have performance impact.
        // This query operation will happen lazily, so it won't cost you anything if the parameter is never queried.
        [[nodiscard]] const str_t& get(const StreamType streamType, const std::size_t streamIndex, const std::size_t parameter, const InfoType infoType = InfoType::Value) const;

    private:
        Parser(const Parser&) = delete;
        Parser& operator=(const Parser&) = delete;

    private:
        friend class ParserPrivate;
        const std::unique_ptr<ParserPrivate> d;
    };

} // namespace MediaInfoLibLite
