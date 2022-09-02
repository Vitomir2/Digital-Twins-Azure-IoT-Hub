// Fill in  your WiFi networks SSID and password
#define SECRET_WIFI_SSID ""
#define SECRET_WIFI_PASS ""

// Azure IoT
#define SECRET_BROKER       "<Azure-IoT-Hub-Name>.azure-devices.net"
#define SECRET_DEVICE_ID    ""
#define SECRET_CONN_STRING  "SharedAccessSignature sr=<Azure-IoT-Hub-Name>.azure-devices.net%2Fdevices%2F<device-ID>-symm&sig=<private-key>%3D&se=<expiration-timestamp>"
// DPS variables
#define SECRET_DPS_BROKER   ""
#define SECRET_ID_SCOPE     ""

// Publish 1 message every 2 seconds
#define TELEMETRY_FREQUENCY_MILLISECS 2000

// Device leaf certificate (if the self-signed certificate is
// not suitable for use). It must be signed by CA Root cert or
// intermediate cert, and made with the device private key.
static const char ROOT_CA_CERT[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDajCCAlKgAwIBAgIQWwjAbNUmFp9FwQHXT6lXwTANBgkqhkiG9w0BAQsFADAo
----------------------------------------------------------------
f08CUH4BibyDPJHXCzVUNn4s3UT+1pQAVZRzCz2EAXMIz2KGNYwW3br/6ubxBTLX
-----END CERTIFICATE-----
)EOF";

static const char CLIENT_CERT[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIICqzCCAZOgAwIBAgIQYR76koAUR4hNpH8e+VqqpjANBgkqhkiG9w0BAQsFADAo
----------------------------------------------------------------
MSYwJAYDVQQDDB1BenVyZSBJb1QgQ0EgVGVzdE9ubHkgUm9vdCBDQTAeFw0yMjA4
-----END CERTIFICATE-----
)EOF";

static const char CLIENT_KEY[] PROGMEM = R"EOF(
-----BEGIN EC PRIVATE KEY-----
MHcCAQEEIDO3dG2VAxrTsQupH3GLTRyWdV6qUbGf+qnHFyJ6WvEvoAoGCCqGSM49
----------------------------------------------------------------
MSYwJAYDVQQDDB1BenVyZSBJb1QgQ0EgVGVzdE9ubHkgUm9vdCBDQTAeFw0yMjA4
-----END EC PRIVATE KEY-----
)EOF";

static const char CLIENT_PUBLIC_CERT[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIICqzCCAZOgAwIBAgIQYR76koAUR4hNpH8e+VqqpjANBgkqhkiG9w0BAQsFADAo
----------------------------------------------------------------
k1AWhKq4HQbZirx4vgmOIlbKIUheBAoARn5uaeQqZbtGH76LfyvbdJR68OP8/+ga
-----END CERTIFICATE-----
)EOF";
