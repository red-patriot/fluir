import subprocess


def build() -> None:
    """Builds the editor backend into an executable"""
    cmd = ["pyinstaller", "--onefile", "--name", "fluir", "fluir_launcher.py"]

    subprocess.run(cmd, check=True)


if __name__ == "__main__":
    build()
