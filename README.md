# C++ Candid Library

The C++ Candid Library of [icpp-pro](https://docs.icpp.world/)

- [API Reference](https://docs.icpp.world/api-reference.html#candidtypes)
- The C++ Candid Library is extensively tested in our [QA Canister](https://github.com/icppWorld/icpp-pro/tree/main/test/canisters/canister_1).
- If you want to contribute, please reach out.

  
## How to use

### C++ Smart Contracts

To write a C++ smart contract that runs in a Canister, you do not use  `icpp-candid` directly, but you install icpp-pro, which is a full-blown C++ Canister Development Kit.

Simply [install](https://docs.icpp.world/installation.html) & [Get Started](https://docs.icpp.world/getting-started.html) !

### C++ Rich Clients

If you want to use the C++ Candid Library in your Rich Client, you need to include it in your application and then compile it to a native executable.

We already do this ourselves as part of `icpp-pro` to enable interactive debugging with VS Code, as described [here](https://docs.icpp.world/getting-started.html#debug-icpp-pro-only).

The command we use is:

```bash
icpp build-native
```

Which uses the Clang++ compiler to create a native debug executable named `mockic.exe` 


## Support

For support, kindly create a GitHub Issue as outlined in the [Support](https://docs.icpp.world/support.html) documentation page.

