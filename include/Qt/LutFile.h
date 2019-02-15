#ifndef LUTFILE_HPP
#define LUTFILE_HPP

#include <QObject>
#include <QVector>
#include <atomic>
#include <mutex>

namespace openshot {
class LutFile : public QObject {
    Q_OBJECT

  public:
    LutFile(QString asset_id, QString filename);

    QString asset_id;
    QString source_file;
    QString lut_title;
    std::atomic_bool initialized;
    QVector<float> lut;
    float domain_min;
    float domain_max;
    int size;

    void initialize();

  private:
    std::mutex lck;
};

class format_exception : public std::runtime_error {
  public:
    explicit format_exception(const std::string &&message)
        : runtime_error(message) {}
};
} // namespace openshot

#endif // LUTFILE_HPP
