from setuptools import setup, Extension

module = Extension(
    'RSA',
    sources=['RSA.c'],
    include_dirs=['/usr/include/python3.11'],
)

setup(
    name='RSA',
    version='1.0',
    description='Python RSA Encryption Module',
    ext_modules=[module]
)