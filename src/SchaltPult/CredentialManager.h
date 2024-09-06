#ifndef CREDENTIAL_MANAGER_H
#define CREDENTIAL_MANAGER_H

class Credential {
  public:
    const char* ssid;   // Wi-Fi SSID
    const char* pass;   // Wi-Fi Password
    const char* host;   // Host Address

    // Constructor to initialize the Credential object
    Credential(const char* ssid, const char* pass, const char* host)
        : ssid(ssid), pass(pass), host(host) {}
};

class CredentialManager {
  public:
    static const Credential testCredential;
    static const Credential productionCredential;
};

/*
define your credentials in Credentials.h like this:

#include "CredentialManager.h"
const Credential CredentialManager::testCredential("***", "***", "***");
const Credential CredentialManager::productionCredential("***", "***", "***");
*/

#endif  // CREDENTIAL_MANAGER_H
