import sys
from PySide6.QtWidgets import QApplication

from screens.splash import SplashScreen


def main() -> int:
    app = QApplication([])

    gui = SplashScreen()
    gui.show()

    return app.exec()


if __name__ == "__main__":
    sys.exit(main())
