#include <cstddef>
#include <ios>
#include <istream>
#include <array>
#include <algorithm>
#include <optional>

namespace pika {
    namespace reversed_stream {
        template<typename IStream, size_t BufferSize = 4096>
        class RStream {
            using char_type = typename IStream::char_type;
            using pos_type = typename IStream::pos_type;
            std::array<char_type, BufferSize> buffer;
            IStream &stream;
            typename std::array<char_type, BufferSize>::iterator cursor, ending;
            pos_type begin;
            void fill() {
                if (!stream.good()) return;
                auto current = stream.tellg();
                auto delta = current - begin >= BufferSize ? BufferSize : current - begin;
                stream.seekg(-delta,  std::ios_base::cur);
                auto read = stream.readsome(buffer.begin(), delta);
                stream.seekg(-read,  std::ios_base::cur);
                ending = buffer.begin() + read;
                std::reverse(buffer.begin(), ending);
                cursor = buffer.begin();
            }

        public:

            explicit RStream(IStream &stream) : stream(stream) {
                buffer.fill(0);
                begin = stream.tellg();
                stream.seekg(0, std::ios_base::end);
                ending = cursor = buffer.end();
            }

            ~RStream() {
                stream.clear();
                stream.seekg(0, std::ios_base::end);
            }

            std::optional<char_type> get() {
                if (cursor == ending) fill();
                if (cursor == ending) return std::nullopt;
                return {*cursor++};
            }
        };
    }
} // namespace pika
