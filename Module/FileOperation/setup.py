from setuptools import setup, Extension

module = Extension(
    'FileOperation',
    sources=['FileOperation.c'],
    include_dirs=['/usr/include/python3.11', '/usr/include/json-c'],
    libraries=['json-c'],
    library_dirs=['/usr/lib/x86_64-linux-gnu'],
)

setup(
    name='FileOperation',
    version='1.0',
    description='Python RSA Encryption Module',
    ext_modules=[module]
)