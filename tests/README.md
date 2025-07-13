# Tests

This directory contains the files used to test the library and the `bin2shellcode` executable.

- `MessageBox.bin` is the shellcode in binary format that is used as the input for the tests. This binary shows a message box and was generated with the following command:

```bash
msfvenom -p windows/x64/messagebox TEXT="Test" TITLE="LoaderOK" -f raw -o MessageBox.bin
```

- `test_converter.cpp` is the test program that reads `MessageBox.bin`, converts the contents to all available formats and compares the results with the files included in this folder.

- `shellcode.*` are the expected representations of the same shellcode in various languages (C, Ruby, Python, C#, Go, Rust, PowerShell) and formats (hex, base64 and uuid). They are used to ensure the conversion is correct.

