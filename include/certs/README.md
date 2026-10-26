# Blynk TLS trust roots

`blynk_roots.h` is an unchanged copy of `src/certs/certs_pem.h` from Blynk library 1.4.0, already installed in this project. Its MIT license is preserved in `LICENSE.Blynk`.

Source: https://github.com/Blynk-Technologies/blynk-library/blob/v1.4.0/src/certs/certs_pem.h

It contains ISRG Root X1, Amazon Root CA 1 and DigiCert Global Root G2. These are public CA certificates, not private keys or Device Tokens. The MQTT firmware uses this bundle with WiFiClientSecure's certificate/hostname validation. It does not use the Blynk binary protocol library.

When Blynk changes its certificate chain or these roots expire, review the official bundle and rebuild the firmware. Do not replace certificate validation with `setInsecure()`.
