import subprocess


def build() -> None:
    """Builds the editor backend into an executable"""
    cmd = ["pyinstaller", "--onefile", "--name", "fluir-editor-be", "main.py"]

    subprocess.run(cmd, check=True)


if __name__ == "__main__":
    build()
