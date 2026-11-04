# C++ Candid Library

The C++ Candid Library of [icpp-pro](https://icpp-pro.onicai.com/)

- [API Reference](https://icpp-pro.onicai.com/api-reference.html#candidtypes)
- It is 100% compliant with the [Candid Interface Specification](https://github.com/dfinity/candid/blob/master/spec/Candid.md) (spec 0.1.8, dfinity/candid@572c80a): see [Candid compliance](https://icpp-pro.onicai.com/candid-compliance.html).
- The C++ Candid Library is extensively tested in our [QA Canister](https://github.com/icppWorld/icpp-pro/tree/main/test/canisters/canister_1).
- If you want to contribute, please reach out.

  
## How to use

### C++ Smart Contracts

To write a C++ smart contract that runs in a Canister, you do not use  `icpp-candid` directly, but you install icpp-pro, which is a full-blown C++ Canister Development Kit.

Simply [install](https://icpp-pro.onicai.com/installation.html) & [Get Started](https://icpp-pro.onicai.com/getting-started.html) !

### C++ Rich Clients

If you want to use the C++ Candid Library in your Rich Client, you need to include it in your application and then compile it to a native executable.

We already do this ourselves as part of `icpp-pro` to enable interactive debugging with VS Code, as described in [debugging with VS Code](https://icpp-pro.onicai.com/getting-started.html#debug-icpp-pro-only).

The command we use is:

```bash
icpp build-native
```

Which uses the Clang++ compiler to create a native debug executable named `mockic.exe` 


## Support

For support, kindly create a GitHub Issue as outlined in the [Support](https://icpp-pro.onicai.com/support.html) documentation page.

