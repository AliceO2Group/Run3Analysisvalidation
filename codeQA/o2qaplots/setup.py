from setuptools import find_packages, setup

setup(
    name="o2qaplots",
    version="1.0.0",
    packages=find_packages(),
    url="https://github.com/hzanoli/O2QA",
    license="MIT License",
    author="Henrique J. C. Zanoli",
    author_email="hzanoli@gmail.com",
    description="Plotting tools for the ALICE O2 quality assurance",
    entry_points={"console_scripts": ["o2qa=o2qaplots.cli:cli"]},
    install_requirepes=["tqdm"],
)
