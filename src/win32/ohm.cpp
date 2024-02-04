/**************************************************************************
 *  ohm.cpp
 *
 *  Copyright 2024 Gabriel Machado
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 **************************************************************************/

#include "ohm.h"

#include <algorithm>

#include <comdef.h>

#include <qdebug.h>
#include <qwinfunctions.h>

template<class T>
static T query_cast(VARIANT *v);

template<>
QString query_cast(VARIANT *v) {
    return QString::fromWCharArray(V_BSTR(v)); }

template<>
float query_cast(VARIANT *v) {
    return V_R4(v); }

template<class T>
static T query_get(IWbemClassObject *obj, LPCWSTR name) {
    VARIANT v;
    obj->Get(name, 0, &v, nullptr, nullptr);
    auto ret = query_cast<T>(&v);
    VariantClear(&v);
    return ret;
}

QDebug operator<<(QDebug dbg, Ohm::HardwareInfo const& hi) {
    dbg.nospace() << "HardwareInfo(id=" << hi.id
                  << ", identifier=" << hi.identifier
                  << ", name=" << hi.name
                  << ", type=" << hi.type << ")";
    return dbg.space();
}

QDebug operator<<(QDebug dbg, Ohm::SensorInfo const& si) {
    dbg.nospace() << "SensorInfo(id=" << si.id
                  << ", identifier=" << si.identifier
                  << ", name=" << si.name
                  << ", type=" << si.type << ")";
    return dbg.space();
}

Ohm::Ohm() {
    m_roleNames[IdRole]    = tr("id").toUtf8();
    m_roleNames[PathRole]  = tr("path").toUtf8();
    m_roleNames[NameRole]  = tr("name").toUtf8();
    m_roleNames[TypeRole]  = tr("type").toUtf8();
    m_roleNames[ValueRole] = tr("value").toUtf8();
    m_roleNames[HwId]      = tr("hwid").toUtf8();
    m_roleNames[HwPath]    = tr("hwpath").toUtf8();
    m_roleNames[HwName]    = tr("hwname").toUtf8();
    m_roleNames[HwType]    = tr("hwtype").toUtf8();

    m_timer.setSingleShot(true);
    connect(&m_timer, &QTimer::timeout, this, &Ohm::update);

    if (initCom() && connectWmi()) {
        m_initialized = true;
        update();
    } else {
        uninit();
    }
}

void Ohm::query(BSTR query, std::function<void(IWbemClassObject *)> callback) {
    HRESULT hr;

    IEnumWbemClassObject *pEnum;
    hr = pSvc->ExecQuery(_bstr_t("WQL"), query, WBEM_FLAG_FORWARD_ONLY, nullptr, &pEnum);

    if (FAILED(hr)) {
       qDebug() << "Ohm::query(): IWbemServices::ExecQuery error:" << hex << unsigned(hr)
                << QtWin::errorStringFromHresult(hr) << QtWin::stringFromHresult(hr);
       goto end;
    }

    while (true) {
        IWbemClassObject *obj = nullptr;
        ULONG count = 0;

        hr = pEnum->Next(0, 1, &obj, &count);

        if (FAILED(hr)) {
           qDebug() << "Ohm::query(): IEnumWbemClassObject::Next error:" << hex << unsigned(hr)
                    << QtWin::errorStringFromHresult(hr) << QtWin::stringFromHresult(hr);
           goto end;
        }

        if (count == 0)
            break;

        callback(obj);

        obj->Release();
    }

end:
    if (pEnum) pEnum->Release();
}

bool Ohm::queryHardware() {
    qDebug() << "Ohm::queryHardware()";

    if (!m_sensors.empty()) {
        beginRemoveRows({}, 0, m_sensors.size() - 1);
        m_hardware.clear();
        m_sensors.clear();
        m_sensorId.clear();
        endRemoveRows();
    }

    decltype(m_hardware) hardware;
    decltype(m_sensors)  sensors;

    query(_bstr_t("select HardwareType, Identifier, InstanceId, Name from Hardware"), [&](auto *obj) {
        hardware.push_back({query_get<QString>(obj, L"InstanceId"),
                            query_get<QString>(obj, L"Identifier"),
                            query_get<QString>(obj, L"Name"),
                            query_get<QString>(obj, L"HardwareType")});
    });

    if (hardware.empty())
        return false;

    query(_bstr_t("select InstanceId, Identifier, Name, SensorType, Parent from Sensor"), [&](auto *obj) {
        auto id = query_get<QString>(obj, L"InstanceId");
        auto path = query_get<QString>(obj, L"Identifier");
        auto name = query_get<QString>(obj, L"Name");
        auto type = query_get<QString>(obj, L"SensorType");
        auto parent = query_get<QString>(obj, L"Parent");

        if (type == "Data")
            type = "GB";
        else if (type == "SmallData")
            type = "MB";

        sensors.push_back({id, path, name, type, parent});
    });

    std::sort(hardware.begin(), hardware.end(),
              [](auto const& lhs, auto const& rhs) { return lhs.id < rhs.id; });
    std::sort(sensors.begin(), sensors.end(),
              [](auto const& lhs, auto const& rhs) { return lhs.id < rhs.id; });

    beginInsertRows({}, 0, sensors.size() - 1);
    m_hardware = std::move(hardware);
    m_sensors = std::move(sensors);
    m_sensorData = QVector<qreal>(m_sensors.size(), 0.0);
    for (int i = 0; i < m_sensors.size(); ++i)
        m_sensorId[m_sensors[i].identifier] = i;
    endInsertRows();

    emit indexChanged();

    return true;
}

bool Ohm::querySensors() {
    int count = 0;

    query(_bstr_t("select Identifier, Value from Sensor"), [this, &count](auto *obj) {
        int i = m_sensorId[query_get<QString>(obj, L"Identifier")];
        m_sensorData[i] = qreal(query_get<float>(obj, L"Value"));
        ++count;
    });

    if (count == m_sensors.size()) {
        emit dataChanged(createIndex(0, 0), createIndex(m_sensorData.size() - 1, 0), {ValueRole});
        return true;
    }

    return false;
}

void Ohm::update() {
    if (! m_initialized)
        return;

    if ((m_sensors.size() > 0 && querySensors())
            || (queryHardware() && querySensors())) {
        if (m_autoUpdate)
            m_timer.start(m_updateInterval);
    } else if (m_autoUpdate)
        m_timer.start(5000);
}

QModelIndex Ohm::index(int row, int column, const QModelIndex &/*parent*/) const  {
    return createIndex(row, column);
}

QModelIndex Ohm::parent(const QModelIndex &/*child*/) const {
    return {};
}

int Ohm::rowCount(const QModelIndex &/*parent*/) const {
    return m_sensors.size();
}

int Ohm::columnCount(const QModelIndex &/*parent*/) const {
    return 1;
}

QVariant Ohm::data(const QModelIndex &index, int role) const {
    auto hardware = m_hardware.begin();
    if (role >= HwId) {
        hardware = std::find_if(m_hardware.begin(), m_hardware.end(),
                              [this, &index](const HardwareInfo &i){
            return i.identifier == m_sensors[index.row()].parent; });
        Q_ASSERT(hardware != m_hardware.end());
    }

    switch (role) {
    case ValueRole: return m_sensorData[index.row()];
    case IdRole:    return m_sensors[index.row()].id;
    case PathRole:  return m_sensors[index.row()].identifier;
    case NameRole:  return m_sensors[index.row()].name;
    case TypeRole:  return m_sensors[index.row()].type;
    case HwId:      return hardware->id;
    case HwPath:    return hardware->identifier;
    case HwName:    return hardware->name;
    case HwType:    return hardware->type;
    }

    Q_ASSERT(false);
    return {};
}

QHash<int, QByteArray> Ohm::roleNames() const {
    return m_roleNames;
}

bool Ohm::initCom() {
    HRESULT hr;

    hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);

    if (hr == RPC_E_CHANGED_MODE) {
        // COM already initialized, have faith :D

        qDebug () << "Ohm::initCom(): CoInitializeEx may have already been called";
        m_uninitializeCom = false;
        return true;
    } else if (FAILED(hr)) {
        qDebug() << "Ohm::initCom(): CoInitializeEx error:" << hex << unsigned(hr)
                 << QtWin::errorStringFromHresult(hr) << QtWin::stringFromHresult(hr);
      return false;
    } else
        m_uninitializeCom = true;

    hr = CoInitializeSecurity(
        nullptr,                     // Security descriptor
        -1,                          // COM negotiates authentication service
        nullptr,                     // Authentication services
        nullptr,                     // Reserved
        RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication level for proxies
        RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation level for proxies
        nullptr,                     // Authentication info
        EOAC_NONE,                   // Additional capabilities of the client or server
        nullptr);                    // Reserved

    if (FAILED(hr)) {
       qDebug() << "Ohm::initCom(): CoInitializeSecurity error:" << hex << unsigned(hr)
                << QtWin::errorStringFromHresult(hr) << QtWin::stringFromHresult(hr);
       CoUninitialize();
       return false;
    }

    return true;
}

bool Ohm::connectWmi() {
    HRESULT hr;

    hr = CoCreateInstance(CLSID_WbemLocator, nullptr,
                          CLSCTX_INPROC_SERVER, IID_IWbemLocator,
                          reinterpret_cast<LPVOID *>(&pLoc));
    if (FAILED(hr)) {
        qDebug() << "Ohm::connectWmi(): CoCreateInstance error:" << hex << unsigned(hr)
                 << QtWin::errorStringFromHresult(hr) << QtWin::stringFromHresult(hr);
        return false;
    }

    // Connect to the root\default namespace with the current user.
    hr = pLoc->ConnectServer(
                _bstr_t("root/openhardwaremonitor"),  //namespace
                nullptr,  // User name
                nullptr,  // User password
                nullptr,  // Locale
                0,        // Security flags
                nullptr,  // Authority
                nullptr,  // Context object
                &pSvc);   // IWbemServices proxy

    if (FAILED(hr)) {
        qDebug() << "Ohm::connectWmi(): IWbemLocator::ConnectServer error:" << hex << unsigned(hr)
                 << QtWin::errorStringFromHresult(hr) << QtWin::stringFromHresult(hr);
        return false;
    }

    // Set the proxy so that impersonation of the client occurs.
    hr = CoSetProxyBlanket(
                pSvc,
                RPC_C_AUTHN_WINNT,
                RPC_C_AUTHZ_NONE,
                nullptr,
                RPC_C_AUTHN_LEVEL_CALL,
                RPC_C_IMP_LEVEL_IMPERSONATE,
                nullptr,
                EOAC_NONE);

    if (FAILED(hr)) {
        qDebug() << "Ohm::connectWmi(): CoSetProxyBlanket error:" << hex << unsigned(hr)
                 << QtWin::errorStringFromHresult(hr) << QtWin::stringFromHresult(hr);
        return false;
    }

    return true;
}

void Ohm::uninit() {
    m_initialized = false;

    if (pSvc) pSvc->Release();
    pSvc = nullptr;

    if (pLoc) pLoc->Release();
    pLoc = nullptr;

    if (m_uninitializeCom) CoUninitialize();
    m_uninitializeCom = false;
}
