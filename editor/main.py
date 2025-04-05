import sys
from editor.splash import SplashScreen
from PySide6.QtWidgets import QApplication


def main() -> int:
    app = QApplication([])

    gui = SplashScreen()
    gui.show()

    return app.exec()


if __name__ == "__main__":
    sys.exit(main())
