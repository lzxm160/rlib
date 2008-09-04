@echo off
python.exe -c "from distutils import sysconfig; import sys; sys.stdout.write(sysconfig.get_python_lib(1,0,prefix=sysconfig.get_config_var('exec_prefix')))"
