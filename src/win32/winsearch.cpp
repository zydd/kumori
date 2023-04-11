#include "winsearch.h"

//#include <atldbcli.h>
#include <SearchAPI.h>

#include <qdebug.h>
#include <qwinfunctions.h>

WinSearch::WinSearch() {
    /*
    ISearchManager* pSearchManager;
    HRESULT hr;
    hr = CoCreateInstance(CLSID_CSearchManager, nullptr, CLSCTX_LOCAL_SERVER, IID_PPV_ARGS(&pSearchManager));
    if (FAILED(hr)) {
       qDebug() << "" << hex << unsigned(hr)
                << QtWin::errorStringFromHresult(hr) << QtWin::stringFromHresult(hr);
       return ;
    }

    ISearchCatalogManager *pSearchCatalogManager;
    hr = pSearchManager->GetCatalog(L"SystemIndex", &pSearchCatalogManager);
    if (FAILED(hr)) {
       qDebug() << "" << hex << unsigned(hr)
                << QtWin::errorStringFromHresult(hr) << QtWin::stringFromHresult(hr);
       return;
    }

    ISearchQueryHelper *pQueryHelper;
    hr = pSearchCatalogManager->GetQueryHelper(&pQueryHelper);
    if (FAILED(hr)) {
       qDebug() << "" << hex << unsigned(hr)
                << QtWin::errorStringFromHresult(hr) << QtWin::stringFromHresult(hr);
       return;
    }

    LPWSTR pszConnectionString = nullptr;
    hr = pQueryHelper->get_ConnectionString(&pszConnectionString);
    // NOTE: YOU MUST call CoTaskMemFree() on the string
    if (FAILED(hr)) {
       qDebug() << "" << hex << unsigned(hr)
                << QtWin::errorStringFromHresult(hr) << QtWin::stringFromHresult(hr);
       return;
    }

    LPWSTR pszQueryString = nullptr;
    hr = pQueryHelper->GenerateSQLFromUserQuery(L"errorStringFromHresult", &pszQueryString);
    if (FAILED(hr)) {
       qDebug() << "" << hex << unsigned(hr)
                << QtWin::errorStringFromHresult(hr) << QtWin::stringFromHresult(hr);
       return;
    }

    qDebug() << QString::fromWCharArray(pszQueryString);
    qDebug() << QString::fromWCharArray(pszConnectionString);

    auto querys = QStringLiteral("SELECT System.ItemUrl FROM SystemIndex WHERE"
                                 " System.ItemName = 'NSIS.lnk'"
//                                 " SCOPE='file:%2/Microsoft/Windows/Start Menu/Programs'"
//                                 " OR SCOPE='file:%2/Microsoft/Windows/Start Menu/Programs'"
                                 )
//            .arg(QString::fromLocal8Bit(qgetenv("ProgramData")))
            .arg(QString::fromLocal8Bit(qgetenv("AppData")));
    auto query = new wchar_t[size_t(querys.size()) + 1];
    query[size_t(querys.size())] = '\0';
    querys.toWCharArray(query);
    qDebug() << QString::fromWCharArray(query);


    CDataSource cDataSource;
    hr = cDataSource.OpenFromInitializationString(pszConnectionString);
    if (FAILED(hr)) {
       qDebug() << "" << hex << unsigned(hr)
                << QtWin::errorStringFromHresult(hr) << QtWin::stringFromHresult(hr);
       return;
    }


    CSession cSession;
    hr = cSession.Open(cDataSource);
    if (FAILED(hr)) {
       qDebug() << "" << hex << unsigned(hr)
                << QtWin::errorStringFromHresult(hr) << QtWin::stringFromHresult(hr);
       return;
    }

    CCommand<CDynamicAccessor, CRowset> cCommand;
    hr = cCommand.Open(cSession, query);
    if (FAILED(hr)) {
       qDebug() << "" << hex << unsigned(hr)
                << QtWin::errorStringFromHresult(hr) << QtWin::stringFromHresult(hr);
       return;
    }

    for (hr = cCommand.MoveFirst(); hr == S_OK; hr = cCommand.MoveNext()) {
        auto dbg = qDebug();
        for (DBORDINAL i = 1; i <= cCommand.GetColumnCount(); i++) {
            PCWSTR pszName = cCommand.GetColumnName(i);
            DBTYPE type = {0};
            cCommand.GetColumnType(i, &type);
            DBLENGTH len = {0};
            cCommand.GetLength(i, &len);

            Q_ASSERT(type == DBTYPE_WSTR);

            auto val = QString::fromWCharArray(reinterpret_cast<wchar_t *>(cCommand.GetValue(i)));


            dbg << QString::fromWCharArray(pszName) << val;
        }
    }
    cCommand.Close();
    */
}
