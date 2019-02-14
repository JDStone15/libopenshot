#include "../include/LutFile.hpp"
#include "../include/utilities/string_helpers.hpp"
#include <QColor>
#include <QFile>
#include <QTextStream>
#include <spdlog/fmt/fmt.h>

using namespace std;
using namespace fmt;

namespace openshot {
LutFile::LutFile(QString asset_id, QString filename)
    : asset_id(asset_id), source_file(filename), initialized(false), size(0) {
    domain_min = 0.0f;
    domain_max = 1.0f;
}

void LutFile::initialize() {
    if (initialized.load()) {
        return;
    } // Double-checked locking... First check

    lock_guard<mutex> lock(lck);

    if (initialized.load()) {
        return;
    } // Double-checked locking... Second check

    QFile cube(source_file);
    cube.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream in(&cube);

    uint line_num = 0;
    do {
        line_num++;
        auto line = in.readLine();

        if (line.startsWith("#") || line.isEmpty()) {
            continue;
        }

        auto split = line.splitRef(" ", QString::SplitBehavior::SkipEmptyParts);

        // I don't think blank lines are support but *shrug*
        if (split.length() == 0) {
            continue;
        }

        auto lede = split[0];
        if (lede.startsWith("TITLE", Qt::CaseInsensitive)) {
            auto const title =
                line.splitRef("\"", QString::SplitBehavior::SkipEmptyParts);
            if (split.length() < 2) {
                throw format_exception(
                    "Invalid TITLE. Received invalid title {}. Line: {}; File: {}"_format(
                        split.length(), line_num, source_file));
            }

            if (!(split[1].startsWith("\"") && split.last().endsWith("\""))) {
                throw format_exception(
                    "Invalid TITLE. Expected quoted string but got '{}'; Line: {}; File: {}"_format(
                        line, line_num, source_file));
            }

            lut_title = split[1].toString().mid(1, split[1].length() - 2);
            continue;
        }

        if (lede.startsWith("LUT_3D_SIZE", Qt::CaseInsensitive)) {
            if (split.length() != 2) {
                throw format_exception(
                    "Invalid LUT_3D_SIZE. Received invalid line length {}. Line: {}; File: {}"_format(
                        split.length(), line_num, source_file));
            }

            auto success = true;
            size = split[1].toInt(&success);

            if (!success) {
                throw format_exception(
                    "Invalid LUT_3D_SIZE. Received invalid size value {}. Line: {}; File: {}"_format(
                        split.length(), line_num, source_file));
            }

            lut.reserve(size * size * size * 3);
            continue;
        }

        if (lede.startsWith("DOMAIN_MIN", Qt::CaseInsensitive)) {
            if (split.length() != 2 && split.length() != 4) {
                throw format_exception(
                    "Invalid DOMAIN_MIN. Received invalid line length {}. Line: {}; File: {}"_format(
                        split.length(), line_num, source_file));
            }

            auto success = true;
            // The standard supports three different domain values for each
            // column, but we don't
            domain_min = split[1].toFloat(&success);

            if (!success) {
                throw format_exception(
                    "Invalid DOMAIN_MIN. Received invalid domain value {}. Line: {}; File: {}"_format(
                        split.length(), line_num, source_file));
            }
            continue;
        }

        if (lede.startsWith("DOMAIN_MAX", Qt::CaseInsensitive)) {
            if (split.length() != 2 && split.length() != 4) {
                throw format_exception(
                    "Invalid DOMAIN_MAX. Received invalid line length {}. Line: {}; File: {}"_format(
                        split.length(), line_num, source_file));
            }

            auto success = true;
            // The standard supports three different domain values for each
            // column, but we don't
            domain_max = split[1].toFloat(&success);

            if (!success) {
                throw format_exception(
                    "Invalid DOMAIN_MAX. Received invalid domain value {}. Line: {}; File: {}"_format(
                        split.length(), line_num, source_file));
            }
            continue;
        }

        if (split.length() != 3) {
            throw format_exception(
                "Invalid LUT row. Received invalid line length {}. Line: {}; File: {}"_format(
                    split.length(), line_num, source_file));
        }

        for (auto i = 0; i < 3; i++) {
            auto success = true;
            lut.push_back(split[i].toFloat(&success));

            if (!success) {
                throw format_exception(
                    "Invalid LUT row. Received invalid domain value {}. Line: {}; File: {}"_format(
                        split.length(), line_num, source_file));
            }
        }

    } while (!in.atEnd());

    if (lut.size() != size * size * size * 3) {
        throw format_exception(
            "Invalid cube file. Received invalid number of rows {}. Expected {}. Line: {}; File: {}"_format(
                lut.size() / 3, size * size * size * 3, line_num, source_file));
    }

    initialized.store(true);
}
} // namespace ripper
