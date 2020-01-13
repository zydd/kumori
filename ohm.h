#ifndef OHM_H
#define OHM_H

#include <functional>

#include <WbemIdl.h>

#include <qabstractitemmodel.h>
#include <qmap.h>
#include <qtimer.h>
#include <qvector.h>

class Ohm : public QAbstractItemModel {
    Q_OBJECT
    Q_PROPERTY(bool autoUpdate READ autoUpdate WRITE setAutoUpdate NOTIFY autoUpdateChanged)
    Q_PROPERTY(int updateInterval READ updateInterval WRITE setUpdateInterval NOTIFY updateIntervalChanged)

public:
    Ohm();
    virtual ~Ohm() override { uninit(); }

private:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        PathRole,
        NameRole,
        TypeRole,
        ValueRole
    };
    struct HardwareInfo {
        QString id;
        QString identifier;
        QString name;
        QString type;
    };
    struct SensorInfo {
        QString id;
        QString identifier;
        QString name;
        QString type;
    };

    QHash<int, QByteArray> m_roleNames;

    IWbemLocator *pLoc = nullptr;
    IWbemServices *pSvc = nullptr;
    bool m_uninitializeCom = false;
    bool m_initialized = false;

    bool m_autoUpdate = true;
    int m_updateInterval = 2000;

    QTimer m_timer;

    QVector<HardwareInfo> m_hardware;
    QVector<SensorInfo> m_sensors;
    QVector<float> m_sensorData;
    QMap<QString, int> m_sensorId;

    friend QDebug operator<<(QDebug dbg, Ohm::HardwareInfo const& hi);
    friend QDebug operator<<(QDebug dbg, Ohm::SensorInfo const& si);

    bool initCom();
    bool connectWmi();
    void uninit();

    void query(BSTR query, std::function<void(IWbemClassObject *)> callback);
    bool queryHardware();
    bool querySensors();

private slots:
    void update();

    // QAbstractItemModel interface
public:
    QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    // Properties
public:
    bool autoUpdate() const {
        return m_autoUpdate; }
    int updateInterval() const {
        return m_updateInterval; }

public slots:
    void setAutoUpdate(bool autoUpdate) {
        if (m_autoUpdate == autoUpdate)
            return;

        m_autoUpdate = autoUpdate;

        if (autoUpdate)
            update();
        else
            m_timer.stop();

        emit autoUpdateChanged(m_autoUpdate);
    }
    void setUpdateInterval(int updateInterval) {
        if (m_updateInterval == updateInterval)
            return;

        if (m_timer.isActive())
            m_timer.setInterval(updateInterval);

        m_updateInterval = updateInterval;
        emit updateIntervalChanged(m_updateInterval);
    }

signals:
    void autoUpdateChanged(bool autoUpdate);
    void updateIntervalChanged(int updateInterval);
};

#endif // OHM_H
