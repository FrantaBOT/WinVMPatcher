# WinVMPatcher

WinVMPatcher is a program that allows driver modification on virtual Windows without the need to sign drivers or enable TESTSIGNING.

## Preparation

Before using WinVMPatcher, ensure that you have the following requirements:

- Python 3 installed
- `pefile` module installed
- Rust and Cargo installed

Follow these steps to prepare the environment:

1. Download WinVMPatcher repository
```
git clone --recursive https://github.com/frantabot/winvmpatcher
```

2. Build the memflow-ffi by executing the following commands:
```
cd memflow/memflow-ffi
cargo build --release
cd ../..
```

3. Build the QEMU connector plugin by cloning the memflow-qemu-procfs repository and executing the following commands:
```
git clone https://github.com/memflow/memflow-qemu-procfs
cd memflow-qemu-procfs
cargo build --release --all-features
```
Copy the file `memflow-qemu-procfs/target/release/libmemflow_qemu.so` to the build folder.

4. Build the Windows plugin by cloning the memflow-win32 repository and executing the following commands:
```
git clone https://github.com/memflow/memflow-win32
cd memflow-win32
cargo build --release --all-features
```
Copy the file `memflow-win32/target/release/libmemflow_win32.so` to the build folder.

5. Compile the WinVMPatcher by executing the following commands:
```
make
```

## Usage

### Memory Converting

Some drivers need to be patched on power up. Therefore, we recommend to turn on the patcher before turning on the VM. (Until the VM is powered on, the patcher will throw errors)

To convert file addresses to memory addresses, use the following command:

```python3 memoryConvert.py memory_diff_path module_name```

Example: ```python3 memoryConvert.py wsys-531.41.diff nvlddmkm.sys```

### Memory Patching

To patch memory, use the following command:

```sudo ./winvmpatcher memory_diff_path module_name [arguments]```

Example: ```sudo ./winvmpatcher wsys-531.41.diff_memory nvlddmkm.sys```

or if you want to run the patcher without a sudo, you can use this command:

```sudo setcap 'CAP_SYS_PTRACE=ep' winvmpatcher```

#### Arguments

The following arguments are available for memory patching:

- `-cn`: Connector name ["qemu"]
- `-ca`: Connector argument [""] - Here you can specify the VM name
- `-on`: OS name ["win32"]
- `-oa`: OS argument [""]
- `-im`: Ignore mismatch
- `-ll`: Log level [3] (0 = Off, 1 = Error, 2 = Warn, 3 = Info, 4 = Debug, 5 = Trace)
