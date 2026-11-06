# SHA-256 validation vectors

The unmodified byte-oriented SHA-256 short- and long-message response files are from NIST CAVP CAVS 11.0, generated 2011-03-15. They are test data, not an implementation dependency.

Source: https://csrc.nist.gov/Projects/Cryptographic-Algorithm-Validation-Program/Secure-Hashing

Archive: https://csrc.nist.gov/CSRC/media/Projects/Cryptographic-Algorithm-Validation-Program/documents/shs/shabytetestvectors.zip

SHA-256 custody:

```
929ef80b7b3418aca026643f6f248815913b60e01741a44bba9e118067f4c9b8  shabytetestvectors.zip
75e1cb83994638481808e225b9eb0c1ebd0c232d952ac42b61abce6363be283c  SHA256ShortMsg.rsp
6fac36f37360bcf74ffcf4465c18e30d6d5a04cc90885b901fc3130c16060974  SHA256LongMsg.rsp
```

`tests/test_core_util.c` consumes every `Len`, `Msg`, `MD` record from both response files. The zero-length record's `Msg = 00` is an empty message, not a one-byte zero message. The driver also checks the FIPS 180-4 examples and a separate deterministic C differential including streaming splits.
