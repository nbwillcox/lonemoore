"""Capture first-chance native exception addresses in a newly launched review process."""
import ctypes as c, ctypes.wintypes as w, pathlib, struct
root=pathlib.Path(__file__).resolve().parents[1]
exe=root/'Builds/Windows/DungeonCrawler/Binaries/Win64/DungeonCrawler.exe'
k=c.WinDLL('kernel32',use_last_error=True);d=c.WinDLL('dbghelp',use_last_error=True)
class SI(c.Structure):
    _fields_=[('cb',w.DWORD),('reserved',w.LPWSTR),('desktop',w.LPWSTR),('title',w.LPWSTR),('x',w.DWORD),('y',w.DWORD),('sx',w.DWORD),('sy',w.DWORD),('cx',w.DWORD),('cy',w.DWORD),('fill',w.DWORD),('flags',w.DWORD),('show',w.WORD),('cb2',w.WORD),('reserved2',c.c_void_p),('stdin',w.HANDLE),('stdout',w.HANDLE),('stderr',w.HANDLE)]
class PI(c.Structure):
    _fields_=[('process',w.HANDLE),('thread',w.HANDLE),('pid',w.DWORD),('tid',w.DWORD)]
class DE(c.Structure):
    _fields_=[('code',w.DWORD),('pid',w.DWORD),('tid',w.DWORD),('pad',w.DWORD),('data',c.c_byte*160)]
k.CreateProcessW.argtypes=[w.LPCWSTR,w.LPWSTR,c.c_void_p,c.c_void_p,w.BOOL,w.DWORD,c.c_void_p,w.LPCWSTR,c.POINTER(SI),c.POINTER(PI)]
k.WaitForDebugEvent.argtypes=[c.POINTER(DE),w.DWORD];k.ContinueDebugEvent.argtypes=[w.DWORD,w.DWORD,w.DWORD]
k.OpenThread.argtypes=[w.DWORD,w.BOOL,w.DWORD];k.OpenThread.restype=w.HANDLE
k.GetThreadContext.argtypes=[w.HANDLE,c.c_void_p];k.CloseHandle.argtypes=[w.HANDLE]
k.ReadProcessMemory.argtypes=[w.HANDLE,c.c_void_p,c.c_void_p,c.c_size_t,c.c_void_p]
d.SymInitializeW.argtypes=[w.HANDLE,w.LPCWSTR,w.BOOL];d.SymSetOptions.argtypes=[w.DWORD]
d.SymFromAddr.argtypes=[w.HANDLE,c.c_ulonglong,c.POINTER(c.c_ulonglong),c.c_void_p]
si=SI();si.cb=c.sizeof(SI);si.flags=1;si.show=0;pi=PI()
cmd=c.create_unicode_buffer(f'"{exe}" -IntegrityReview -RenderOffscreen -windowed -nosound')
if not k.CreateProcessW(str(exe),cmd,None,None,False,2,None,str(exe.parent),c.byref(si),c.byref(pi)):raise c.WinError(c.get_last_error())
report=[];symbols=False
def out(s):report.append(s);print(s,flush=True)
for step in range(2000):
    ev=DE()
    if not k.WaitForDebugEvent(c.byref(ev),30000):out('Debug event timed out');break
    status=0x10002
    if ev.code==1:
        raw=bytes(ev.data);code=struct.unpack_from('<I',raw)[0];address=struct.unpack_from('<Q',raw,16)[0]
        if code not in (0x80000003,0x406d1388):
            out(f'Exception {code:08x} at {address:x} thread {ev.tid}')
            if not symbols:
                d.SymSetOptions(0x10|0x4|0x2);symbols=bool(d.SymInitializeW(pi.process,str(exe.parent),True));out(f'Symbol initialization {symbols}')
            th=k.OpenThread(0x48,False,ev.tid);buf=c.create_string_buffer(1232+16);ptr=(c.addressof(buf)+15)&~15;c.c_uint32.from_address(ptr+48).value=0x100003
            if k.GetThreadContext(th,ptr):
                rip=c.c_uint64.from_address(ptr+248).value;rsp=c.c_uint64.from_address(ptr+152).value
                stack=c.create_string_buffer(2048);k.ReadProcessMemory(pi.process,rsp,stack,2048,None)
                addresses=[rip]+list(struct.unpack('<256Q',stack.raw))
                for a in addresses:
                    sb=c.create_string_buffer(2048);c.c_uint32.from_buffer(sb).value=88;c.c_uint32.from_buffer(sb,80).value=1900;disp=c.c_ulonglong()
                    if a>0x10000 and d.SymFromAddr(pi.process,a,c.byref(disp),sb):
                        name=sb.raw[84:].split(b'\0')[0].decode(errors='replace')
                        if disp.value<0x20000:out(f'  {a:x} {name}+0x{disp.value:x}')
            k.CloseHandle(th);status=0x80010001
    elif ev.code==5:
        out('Process exited '+str(struct.unpack_from('<I',bytes(ev.data))[0]));k.ContinueDebugEvent(ev.pid,ev.tid,status);break
    k.ContinueDebugEvent(ev.pid,ev.tid,status)
(root/'Saved/IntegrityUpdate/startup_diagnostic.txt').write_text('\n'.join(report))
k.CloseHandle(pi.thread);k.CloseHandle(pi.process)
