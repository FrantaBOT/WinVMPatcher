# WinVMPatcher

WinVMPatcher is a program that allows driver modification on virtual Windows without the need to sign drivers or enable TESTSIGNING.

## Preparation

Before using WinVMPatcher, ensure that you have the following requirements:

- Python 3 installed
- `pefile` module installed

Follow these steps to prepare the environment:

1. Create a build folder.
2. Build the QEMU connector plugin by cloning the memflow-qemu-procfs repository and executing the following commands:
```
git clone https://github.com/memflow/memflow-qemu-procfs
cd memflow-qemu-procfs
cargo build --release --all-features
```
Copy the file `memflow-qemu-procfs/target/release/libmemflow_qemu.so` to the build folder.

3. Build the Windows plugin by cloning the memflow-win32 repository and executing the following commands:
```
git clone https://github.com/memflow/memflow-win32
cd memflow-win32
cargo build --release --all-features
```
Copy the file `memflow-win32/target/release/libmemflow_win32.so` to the build folder.

4. Build the WinVMPatcher by executing the following commands:
```
cd memflow/memflow-ffi
cargo build --release
cd ../..

make
```

## Usage

### Memory Converting

To convert file addresses to memory addresses, use the following command:

```python3 memoryConvert.py memory_diff_path module_name```

Example: ```python3 memoryConvert.py wsys-531.41.diff nvlddmkm.sys```

### Memory Patching

To patch memory, use the following command:

```sudo ./winvmpatcher memory_diff_path module_name [arguments]```

Example: ```sudo ./winvmpatcher wsys-531.41.diff_memory nvlddmkm.sys```

#### Arguments

The following arguments are available for memory patching:

- `-cn`: Connector name
- `-ca`: Connector argument
- `-on`: OS name
- `-oa`: OS argument
- `-im`: Ignore mismatch
- `-ll`: Log level (0 = Off, 1 = Error, 2 = Warn, 3 = Info, 4 = Debug, 5 = Trace)