#!/usr/bin/env python3
import os
import subprocess
import sys
from pathlib import Path
from typing import Final

import yaml

HELP_TEXT: Final = """The Fluir Programming Language

Usage: fluir <command> [arguments...]
Commands:
  compile  - Compile Fluir source code
  run      - Run Fluir bytecode with the VM
  edit     - Launch the Fluir editor
  help     - Show this help message

Examples:
  fluir compile myfile.fl
  fluir run myfile.flc
  fluir edit"""


class FluirLauncher:
    def __init__(self, script_dir: Path) -> None:
        self.script_dir = script_dir
        self.config_file = self.script_dir / "fluir-config.yaml"
        self.config = self.load_config()

    def load_config(self) -> dict[str, str]:
        """Load configuration from YAML file, create default if missing."""
        if self.config_file.exists():
            try:
                with open(self.config_file, "r") as f:
                    config: dict[str, str] = yaml.safe_load(f) or {}
                return config
            except Exception as e:
                print(f"Error reading config file: {e}")
                raise
        else:
            raise FileNotFoundError(
                f"Config file not found: {self.config_file}"
            )

    def get_executable_path(self, base_path: str) -> str:
        """Get the full executable path with proper extension for the platform."""
        # Convert to absolute path if relative
        if not os.path.isabs(base_path):
            base_path = str(self.script_dir / base_path)

        # Add .exe extension on Windows
        if sys.platform.startswith("win"):
            if not base_path.endswith(".exe"):
                base_path += ".exe"

        return base_path

    def check_executable_exists(self, exe_path: str) -> bool:
        """Check if executable exists and is executable."""
        path = Path(exe_path)
        return path.exists() and (path.is_file() or path.is_dir())

    def run_command(self, exe_path: str, args: list[str]) -> int:
        """Run a command with the given arguments."""
        full_path = self.get_executable_path(exe_path)

        if not self.check_executable_exists(full_path):
            print(f"Error: Executable not found: {full_path}")
            return 1

        try:
            # Run the process and wait for completion
            result = subprocess.run(
                [full_path] + args,
                stdout=None,  # Let output go directly to terminal
                stderr=None,  # Let errors go directly to terminal
                stdin=None,
            )  # Let input come from terminal
            return result.returncode
        except Exception as e:
            print(f"Error running {full_path}: {e}")
            return 1

    def run_editor(self, args: list[str]) -> int:
        """Run both editor components."""
        backend_path = self.get_executable_path(
            self.config["editor_backend_path"]
        )
        frontend_path = self.get_executable_path(
            self.config["editor_frontend_path"]
        )

        # Check both executables exist
        if not self.check_executable_exists(backend_path):
            print(f"Error: Editor backend not found: {backend_path}")
            return 1
        if not self.check_executable_exists(frontend_path):
            print(f"Error: Editor frontend not found: {frontend_path}")
            return 1

        try:
            # Start backend process
            backend_proc = subprocess.Popen([backend_path] + args)

            # Start frontend process
            frontend_proc = subprocess.Popen([frontend_path] + args)

            # Wait for both processes to complete
            frontend_result = frontend_proc.wait()
            print("Stopping BE!")
            backend_proc.terminate()
            backend_result = backend_proc.wait()
            print("BE stopped!")

            # Return non-zero if either process failed
            return backend_result or frontend_result

        except Exception as e:
            print(f"Error running editor: {e}")
            return 1

    def show_help(self) -> None:
        """Show launcher help information."""
        print(HELP_TEXT)

    def main(self, args: list[str]) -> int:
        """Main entry point for the launcher."""
        if len(args) < 1:
            self.show_help()
            return 1

        command = args[0].lower()
        remaining_args = args[1:]

        if command == "help":
            self.show_help()
            return 0
        elif command == "compile":
            return self.run_command(
                self.config["compiler_path"], remaining_args
            )
        elif command == "run":
            return self.run_command(self.config["vm_path"], remaining_args)
        elif command == "edit":
            return self.run_editor(remaining_args)
        else:
            print(f"Error: Unknown command '{command}'")
            print("Use 'fluir help' to see available commands.")
            return 1


def main() -> int:
    """Entry point when run as a script."""
    # TODO: Get the correct directory in windows too
    launcher = FluirLauncher(Path("/usr/share/fluir"))
    return launcher.main(sys.argv[1:])


if __name__ == "__main__":
    sys.exit(main())
