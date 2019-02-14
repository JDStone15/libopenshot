#ifndef UTILITIES_H
#define UTILITIES_H

#include <spdlog/fmt/fmt.h>
#include <QString>
#include <chrono>

static void format_arg(fmt::BasicFormatter<char> &f, const char *&format_str,
                       const QString &qstring) {
    Q_UNUSED(format_str)

    f.writer().write(qstring.toStdString().c_str());
}

/// Credit to Stack Overflow user Yakk
/// https://stackoverflow.com/a/42139394
template <class... Durations, class DurationIn>
std::tuple<Durations...> break_down_durations(DurationIn d) {
    std::tuple<Durations...> retval;
    using discard = int[];
    (void)discard{0, (void(((std::get<Durations>(retval) =
                                 std::chrono::duration_cast<Durations>(d)),
                            (d -= std::chrono::duration_cast<DurationIn>(
                                 std::get<Durations>(retval))))),
                      0)...};
    return retval;
}

#endif // UTILITIES_H
