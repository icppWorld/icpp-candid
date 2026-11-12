# C++ Candid Library

The C++ Candid Library of [icpp-pro](https://icpp-pro.onicai.com/)

- [API Reference](https://icpp-pro.onicai.com/api-reference.html#candidtypes)
- It is 100% compliant with the [Candid Interface Specification](https://github.com/dfinity/candid/blob/572c80a/spec/Candid.md) (spec 0.1.8, dfinity/candid@572c80a), within its [limits](https://icpp-pro.onicai.com/candid-compliance.html#limits): it decodes every message of the spec, and encodes every value, recursive types included, to the bytes of the reference implementation. All 472 asserts of the official suite pass, and all 1190 with the Candid tests of the Rust and Motoko CDKs. See [Candid compliance](https://icpp-pro.onicai.com/candid-compliance.html).
- The C++ Candid Library is extensively tested in our [QA Canister](https://github.com/icppWorld/icpp-pro/tree/main/test/canisters/canister_1).
- Want to contribute? See [Contributing](#contributing).

  
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


## Contributing

icpp-candid is developed and released together with icpp-pro, cloned inside it
as `icpp-pro/icpp-candid/`. Set up with icpp-pro's
[Contributors Guide](https://github.com/icppWorld/icpp-pro/blob/main/README-contributors-guide.md);
the tests and fuzzers of this library live in icpp-pro.

- Work on a branch and open a pull request into `main`. Every pull request
  needs a manual approval from the maintainer before it merges.
- Before each push to a pull request:
  - a local CodeRabbit review of the branch against `main`
    (`coderabbit review --base main --agent`);
  - a code review and a security review of the branch. This library decodes
    untrusted messages, so the security review matters for every change;
  - icpp-pro's `make all-static` and the tests of the affected canisters pass.
- CodeRabbit on GitHub also reviews every push to a pull request. Each of its
  findings gets a fix, or a reply saying why not.
- The pull request lists each review's findings and what was done about them.
- A release gets two deeper reviews on top, as described in icpp-pro's
  [release guide](https://github.com/icppWorld/icpp-pro/blob/main/README-release-guide.md#deep-reviews-of-the-release).

## Support

For support, kindly create a GitHub Issue as outlined in the [Support](https://icpp-pro.onicai.com/support.html) documentation page.

