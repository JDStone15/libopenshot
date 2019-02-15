#include "../../include/Qt/LutFile.h"
#include <QColor>
#include <QFile>
#include <QTextStream>

using namespace std;

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
                    "Invalid TITLE. Received invalid title '" + to_string(split.length()) +
                    "'. Line: " + to_string(line_num) +
                    "; File: " + source_file.toStdString());
            }

            if (!(split[1].startsWith("\"") && split.last().endsWith("\""))) {
                throw format_exception(
                    "Invalid TITLE. Expected quoted string but got '" + line.toStdString() +
                    "'; Line: " + to_string(line_num) +
                    "; File: " + source_file.toStdString());
            }

            lut_title = split[1].toString().mid(1, split[1].length() - 2);
            continue;
        }

        if (lede.startsWith("LUT_3D_SIZE", Qt::CaseInsensitive)) {
            if (split.length() != 2) {
                throw format_exception(
                    "Invalid LUT_3D_SIZE. Received invalid line length " + to_string(split.length()) +
                    ". Line: " + to_string(line_num) + 
                    "; File: " + source_file.toStdString());
            }

            auto success = true;
            size = split[1].toInt(&success);

            if (!success) {
                throw format_exception(
                    "Invalid LUT_3D_SIZE. Received invalid size value " + to_string(split.length()) +
                    ". Line: " + to_string(line_num) + 
                    "; File: " + source_file.toStdString());
            }

            lut.reserve(size * size * size * 3);
            continue;
        }

        if (lede.startsWith("DOMAIN_MIN", Qt::CaseInsensitive)) {
            if (split.length() != 2 && split.length() != 4) {
                throw format_exception(
                    "Invalid DOMAIN_MIN. Received invalid line length " + to_string(split.length()) +
                    ". Line: " + to_string(line_num) + 
                    "; File: " + source_file.toStdString());
            }

            auto success = true;
            // The standard supports three different domain values for each
            // column, but we don't
            domain_min = split[1].toFloat(&success);

            if (!success) {
                throw format_exception(
                    "Invalid DOMAIN_MIN. Received invalid domain value " + to_string(split.length()) +
                    ". Line: " + to_string(line_num) + 
                    "; File: " + source_file.toStdString());
            }
            continue;
        }

        if (lede.startsWith("DOMAIN_MAX", Qt::CaseInsensitive)) {
            if (split.length() != 2 && split.length() != 4) {
                throw format_exception(
                    "Invalid DOMAIN_MAX. Received invalid line length " + to_string(split.length()) +
                    ". Line: " + to_string(line_num) + 
                    "; File: " + source_file.toStdString());
            }

            auto success = true;
            // The standard supports three different domain values for each
            // column, but we don't
            domain_max = split[1].toFloat(&success);

            if (!success) {
                throw format_exception(
                    "Invalid DOMAIN_MIN. Received invalid domain value " + to_string(split.length()) +
                    ". Line: " + to_string(line_num) + 
                    "; File: " + source_file.toStdString());
            }
            continue;
        }

        if (split.length() != 3) {
            throw format_exception(
                "Invalid LUT row. Received invalid line length " + to_string(split.length()) +
                ". Line: " + to_string(line_num) + 
                "; File: " + source_file.toStdString());
        }

        for (auto i = 0; i < 3; i++) {
            auto success = true;
            lut.push_back(split[i].toFloat(&success));

            if (!success) {
                throw format_exception(
                    "Invalid LUT row. Received invalid domain value " + to_string(split.length()) +
                    ". Line: " + to_string(line_num) + 
                    "; File: " + source_file.toStdString());
            }
        }

    } while (!in.atEnd());

    if (lut.size() != size * size * size * 3) {
        throw format_exception(
            "Invalid cube file. Received invalid number of rows " + to_string(lut.size() / 3) + 
            ". Expected " + to_string(size * size * size * 3) + 
            ". Line: " + to_string(line_num) + 
            "; File: " + source_file.toStdString());
    }

    initialized.store(true);
}
} // namespace openshot
