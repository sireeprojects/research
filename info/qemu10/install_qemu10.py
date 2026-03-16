import subprocess
import os

def build_qemu():
    # 1. Setup dynamic paths
    # Get the directory where the user called the script from
    base_dir = os.getcwd()
    install_dir = os.path.join(base_dir, "installed")
    
    source_archive = "qemu-10.0.0.tar.xz"
    source_dir = "qemu-10.0.0"
    jobs = "20"

    try:
        # 2. Extract the tarball using system tar (faster + visual feedback)
        if not os.path.exists(source_archive):
            print(f"Error: {source_archive} not found in {base_dir}")
            return

        print(f"--- Extracting {source_archive} ---")
        subprocess.run(["tar", "xvf", source_archive], check=True)

        # 3. Change directory into the extracted source
        os.chdir(source_dir)

        # 4. Configure
        # We use the absolute path for --prefix to avoid confusion
        print(f"--- Running Configure (Prefix: {install_dir}) ---")
        configure_cmd = [
            "./configure", 
            "--target-list=x86_64-softmmu", 
            f"--prefix={install_dir}"
        ]
        subprocess.run(configure_cmd, check=True)

        # 5. Build
        print(f"--- Running Make with {jobs} jobs ---")
        subprocess.run(["make", "-j", jobs], check=True)

        # 6. Install
        print("--- Running Make Install ---")
        subprocess.run(["make", "install"], check=True)

        print(f"\nSuccess! QEMU installed to: {install_dir}")

    except subprocess.CalledProcessError as e:
        print(f"\nCommand failed with return code {e.returncode}")
    except Exception as e:
        print(f"\nAn error occurred: {e}")

if __name__ == "__main__":
    build_qemu()
