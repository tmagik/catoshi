//
// Unit tests for alert system
//

#include "alert.h"
#include "data/alertTests.raw.h"

#include "serialize.h"
#include "util.h"
#include "version.h"

#include <fstream>

#include <boost/filesystem/operations.hpp>
#include <boost/foreach.hpp>
#include <boost/test/unit_test.hpp>

#if 0
//
// alertTests contains 7 alerts, generated with this code:
// (SignAndSave code not shown, alert signing key is secret)
//
{
    CAlert alert;
    alert.nRelayUntil   = 60;
    alert.nExpiration   = 24 * 60 * 60;
    alert.nID           = 1;
    alert.nCancel       = 0;   // cancels previous messages up to this ID number
    alert.nMinVer       = 0;  // These versions are protocol versions
    alert.nMaxVer       = 999001;
    alert.nPriority     = 1;
    alert.strComment    = "Alert comment";
    alert.strStatusBar  = "Alert 1";

    SignAndSave(alert, "test/alertTests");

    alert.setSubVer.insert(std::string("/Satoshi:0.1.0/"));
    alert.strStatusBar  = "Alert 1 for Satoshi 0.1.0";
    SignAndSave(alert, "test/alertTests");

    alert.setSubVer.insert(std::string("/Satoshi:0.2.0/"));
    alert.strStatusBar  = "Alert 1 for Satoshi 0.1.0, 0.2.0";
    SignAndSave(alert, "test/alertTests");

    alert.setSubVer.clear();
    ++alert.nID;
    alert.nCancel = 1;
    alert.nPriority = 100;
    alert.strStatusBar  = "Alert 2, cancels 1";
    SignAndSave(alert, "test/alertTests");

    alert.nExpiration += 60;
    ++alert.nID;
    SignAndSave(alert, "test/alertTests");

    ++alert.nID;
    alert.nMinVer = 11;
    alert.nMaxVer = 22;
    SignAndSave(alert, "test/alertTests");

    ++alert.nID;
    alert.strStatusBar  = "Alert 2 for Satoshi 0.1.0";
    alert.setSubVer.insert(std::string("/Satoshi:0.1.0/"));
    SignAndSave(alert, "test/alertTests");

    ++alert.nID;
    alert.nMinVer = 0;
    alert.nMaxVer = 999999;
    alert.strStatusBar  = "Evil Alert'; /bin/ls; echo '";
    alert.setSubVer.clear();
    SignAndSave(alert, "test/alertTests");
}
#endif

struct ReadAlerts
{
    ReadAlerts()
    {
        std::vector<unsigned char> vch(alert_tests::alertTests, alert_tests::alertTests + sizeof(alert_tests::alertTests));
        CDataStream stream(vch, SER_DISK, CLIENT_VERSION);
        try {
            while (stream.good())
            {
                CAlert alert;
                stream >> alert;
                alerts.push_back(alert);
            }
        }
        catch (std::exception) { }
    }
    ~ReadAlerts() { }

    static std::vector<std::string> read_lines(boost::filesystem::path filepath)
    {
        std::vector<std::string> result;

        std::ifstream f(filepath.string().c_str());
        std::string line;
        while (std::getline(f,line))
            result.push_back(line);

        return result;
    }

    std::vector<CAlert> alerts;
};

BOOST_FIXTURE_TEST_SUITE(Alert_tests, ReadAlerts)


BOOST_AUTO_TEST_CASE(AlertApplies)
{
    SetMockTime(11);

    BOOST_FOREACH(const CAlert& alert, alerts)
    {
        BOOST_CHECK(alert.CheckSignature());
    }

    BOOST_CHECK(alerts.size() >= 3);

    // Matches:
    BOOST_CHECK(alerts[0].AppliesTo(1, ""));
    BOOST_CHECK(alerts[0].AppliesTo(999001, ""));
    BOOST_CHECK(alerts[0].AppliesTo(1, "/Satoshi:11.11.11/"));

    BOOST_CHECK(alerts[1].AppliesTo(1, "/Satoshi:0.1.0/"));
    BOOST_CHECK(alerts[1].AppliesTo(999001, "/Satoshi:0.1.0/"));

    BOOST_CHECK(alerts[2].AppliesTo(1, "/Satoshi:0.1.0/"));
    BOOST_CHECK(alerts[2].AppliesTo(1, "/Satoshi:0.2.0/"));

    // Don't match:
    BOOST_CHECK(!alerts[0].AppliesTo(-1, ""));
    BOOST_CHECK(!alerts[0].AppliesTo(999002, ""));

    BOOST_CHECK(!alerts[1].AppliesTo(1, ""));
    BOOST_CHECK(!alerts[1].AppliesTo(1, "Satoshi:0.1.0"));
    BOOST_CHECK(!alerts[1].AppliesTo(1, "/Satoshi:0.1.0"));
    BOOST_CHECK(!alerts[1].AppliesTo(1, "Satoshi:0.1.0/"));
    BOOST_CHECK(!alerts[1].AppliesTo(-1, "/Satoshi:0.1.0/"));
    BOOST_CHECK(!alerts[1].AppliesTo(999002, "/Satoshi:0.1.0/"));
    BOOST_CHECK(!alerts[1].AppliesTo(1, "/Satoshi:0.2.0/"));

    BOOST_CHECK(!alerts[2].AppliesTo(1, "/Satoshi:0.3.0/"));

    SetMockTime(0);
}


// This uses sh 'echo' to test the -alertnotify function, writing to a
// /tmp file. So skip it on Windows:
#ifndef WIN32
BOOST_AUTO_TEST_CASE(AlertNotify)
{
    SetMockTime(11);

    boost::filesystem::path temp = GetTempPath() / "alertnotify.txt";
    boost::filesystem::remove(temp);

    mapArgs["-alertnotify"] = std::string("echo %s >> ") + temp.string();

    BOOST_FOREACH(CAlert alert, alerts)
        alert.ProcessAlert(false);

    std::vector<std::string> r = read_lines(temp);
    BOOST_CHECK_EQUAL(r.size(), 4u);
    BOOST_CHECK_EQUAL(r[0], "Alert 1");
    BOOST_CHECK_EQUAL(r[1], "Alert 2, cancels 1");
    BOOST_CHECK_EQUAL(r[2], "Alert 2, cancels 1");
    BOOST_CHECK_EQUAL(r[3], "Evil Alert; /bin/ls; echo "); // single-quotes should be removed

    boost::filesystem::remove(temp);

    SetMockTime(0);
}
#endif

BOOST_AUTO_TEST_SUITE_END()
