#include "MediaInfoLibLite.h"
#include "MediaInfo.h"
#include "MediaInfo_Config.h"
#include <cassert>
#include <optional>
#include <unordered_map>

namespace MediaInfoLibLite::details {

    [[nodiscard]] static constexpr inline MediaInfoLib::stream_t toMediaInfoLib(const StreamType streamType) {
        switch (streamType) {
            case StreamType::General:
                return MediaInfoLib::stream_t::Stream_General;
            case StreamType::Video:
                return MediaInfoLib::stream_t::Stream_Video;
            case StreamType::Audio:
                return MediaInfoLib::stream_t::Stream_Audio;
            case StreamType::Subtitle:
                return MediaInfoLib::stream_t::Stream_Text;
            case StreamType::Chapter:
                return MediaInfoLib::stream_t::Stream_Menu;
        }
    }

    [[nodiscard]] static constexpr inline MediaInfoLib::info_t toMediaInfoLib(const InfoType infoType) {
        switch (infoType) {
            case InfoType::Value:
                return MediaInfoLib::info_t::Info_Text;
            case InfoType::Title:
                return MediaInfoLib::info_t::Info_Name;
        }
    }

    template <typename T>
    [[nodiscard]] static inline std::size_t hashCombine(const T& t, const std::size_t seed) noexcept {
        // Combiner taken from N3876 / boost::hash_combine
        return seed ^ (std::hash<T>{}(t) + 0x9e3779b9 + (seed << 6) + (seed >> 2));
    }

    struct Key final {
        StreamType streamType{ StreamType::General };
        std::size_t streamIndex{ 0 };
        std::size_t parameter{ 0 };
        InfoType infoType{ InfoType::Value };
    };

    [[nodiscard]] static inline bool operator==(const Key& lhs, const Key& rhs) {
        return lhs.streamType == rhs.streamType && lhs.streamIndex == rhs.streamIndex && lhs.parameter == rhs.parameter && lhs.infoType == rhs.infoType;
    }

    [[nodiscard]] static inline bool operator!=(const Key& lhs, const Key& rhs) {
        return !operator==(lhs, rhs);
    }

    struct KeyEQU final {
        [[nodiscard]] inline bool operator()(const Key& lhs, const Key& rhs) const {
            return operator==(lhs, rhs);
        }
    };

    struct KeyNEQ final {
        [[nodiscard]] inline bool operator()(const Key& lhs, const Key& rhs) const {
            return operator!=(lhs, rhs);
        }
    };

    struct KeyHash final {
        [[nodiscard]] inline std::size_t operator()(const Key& key) const {
            std::size_t result{ 0 };
            result = hashCombine(static_cast<std::underlying_type_t<decltype(key.streamType)>>(key.streamType), result);
            result = hashCombine(key.streamIndex, result);
            result = hashCombine(key.parameter, result);
            result = hashCombine(static_cast<std::underlying_type_t<decltype(key.infoType)>>(key.infoType), result);
            return result;
        }
    };

} // namespace MediaInfoLibLite::details

namespace std {

    template <>
    struct equal_to<MediaInfoLibLite::details::Key> final {
        [[nodiscard]] inline bool operator()(const MediaInfoLibLite::details::Key& lhs, const MediaInfoLibLite::details::Key& rhs) const {
            return operator==(lhs, rhs);
        }
    };

    template <>
    struct not_equal_to<MediaInfoLibLite::details::Key> final {
        [[nodiscard]] inline bool operator()(const MediaInfoLibLite::details::Key& lhs, const MediaInfoLibLite::details::Key& rhs) const {
            return operator!=(lhs, rhs);
        }
    };

    template <>
    struct hash<MediaInfoLibLite::details::Key> final {
        [[nodiscard]] inline std::size_t operator()(const MediaInfoLibLite::details::Key& key) const {
            return MediaInfoLibLite::details::KeyHash{}(key);
        }
    };

} // namespace std

namespace MediaInfoLibLite {

    class ParserPrivate final {
        ParserPrivate(const ParserPrivate&) = delete;
        ParserPrivate& operator=(const ParserPrivate&) = delete;

    public:
        ParserPrivate(Parser* qq);
        ~ParserPrivate();

        Parser* q_ptr{ nullptr };
        str_t filePath{};
        bool valid{ false };
        std::optional<str_t> informText{ std::nullopt };
        std::unordered_map<details::Key, std::optional<str_t>> valueMap{};
        std::unique_ptr<MediaInfoLib::MediaInfo> mi;
        static inline const str_t emptyStr{};
    };

    ParserPrivate::ParserPrivate(Parser* qq) : q_ptr{ qq } { assert(q_ptr); }

    ParserPrivate::~ParserPrivate() = default;

    Parser::Parser() : d{ std::make_unique<ParserPrivate>(this) } {}

    Parser::Parser(str_t path) : Parser() {
        setFilePath(std::move(path));
    }

    Parser::~Parser() = default;

    const str_t& Parser::filePath() const {
        return d->filePath;
    }

    void Parser::setFilePath(str_t path) {
        assert(!path.empty());
        if (path.empty()) {
            return;
        }
        if (d->filePath == path && d->valid) {
            return;
        }
        if (d->filePath != path) {
            reset();
            d->filePath = std::move(path);
        }
        if (!d->mi) {
            d->mi = std::make_unique<MediaInfoLib::MediaInfo>();
            MediaInfoLib::Config.Cover_Data_Set("base64");
        }
        d->valid = d->mi->Open(d->filePath);
        if (!d->valid) {
            d->mi.reset();
        }
    }

    bool Parser::isValid() const {
        return d->valid;
    }

    void Parser::reset() {
        d->filePath.clear();
        d->valid = false;
        d->informText.reset();
        d->valueMap.clear();
        d->mi.reset();
    }

    const str_t& Parser::inform() const {
        assert(d->valid);
        if (!d->valid) {
            return ParserPrivate::emptyStr;
        }
        assert(d->mi);
        if (!d->informText.has_value()) {
            d->informText = std::move(d->mi->Inform());
        }
        return d->informText.value();
    }

    const str_t& Parser::get(const StreamType streamType, const std::size_t streamIndex, const std::size_t parameter, const InfoType infoType) const {
        assert(d->valid);
        if (!d->valid) {
            return ParserPrivate::emptyStr;
        }
        assert(d->mi);
        auto& result{ d->valueMap[details::Key{ streamType, streamIndex, parameter, infoType }] };
        if (!result.has_value()) {
            result = std::move(d->mi->Get(details::toMediaInfoLib(streamType), streamIndex, parameter, details::toMediaInfoLib(infoType)));
        }
        return result.value();
    }

} // namespace MediaInfoLibLite
