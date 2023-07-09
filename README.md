# C++ Candid Library

The C++ Candid Library of [icpp](https://docs.icpp.world/)

- [API Reference](https://docs.icpp.world/api-reference.html#candidtypes)
- The C++ Candid Library is extensively tested in our [QA Canister](https://github.com/icppWorld/icpp-free/tree/main/test/canisters/canister_1). *(\*)*



# How to use

## C++ Smart Contracts

To write a C++ smart contract that runs in a Canister, you do not use  `icpp-candid` directly, but you install icpp, which is a full-blown C++ Canister Development Kit.

Simply [install](https://docs.icpp.world/installation.html) & [Get Started](https://docs.icpp.world/getting-started.html) !

## C++ Rich Clients

If you want to use the C++ Candid Library in your Rich Client, you need to include it in your application and then compile it to a native executable.

We already do this ourselves as part of `icpp-pro` to enable interactive debugging with VS Code, as described [here](https://docs.icpp.world/getting-started.html#debug-icpp-pro-only).

The command we use is:

```bash
icpp build-native
```

Which uses the Clang++ compiler to create a native debug executable named `mockic.exe` 

During that compilation to a native executable, we use a `MockIC` module, because we mock how the code would run inside a canister.

When you build a Rich Client, you will have to provide some alternative code for that `MockIC` module. Likely just stub it out. 



# Roadmap

## C++ Agent

It is on our road-map to make the building of C++ Rich Clients easier by providing an actual C++ agent. 

No date is available yet.

## Compiler support

Currently we only support the Clang++ compiler. It is **not** on our road-map to support other compilers for building Rich Clients, but let us know if you need that.

# icpp community

Please [join us on OpenChat](https://oc.app/group/tjvrf-oqaaa-aaaar-ahp6a-cai/?ref=6e3y2-4yaaa-aaaaf-araya-cai&code=9104f504fcd17b1f) and let us know your needs, to help us prioritize our road-map.



# Notes

*(\*)* *We currently run the QA tests in the private repository of `icpp-pro`. We are working on migrating this GitHub Action work-flow to the open source version `icpp-free` as part of our road-map.*
