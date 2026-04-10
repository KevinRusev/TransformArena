import subprocess, sys, os

def run(cmd, **kw):
    r = subprocess.run(cmd, capture_output=True, text=True, cwd=r"c:\Users\user\Downloads\assignment", **kw)
    if r.returncode != 0:
        print(f"ERR: {r.stderr}")
        sys.exit(1)
    return r.stdout.strip()

def commit(msg, date):
    run(["git", "add", "-A"])
    tree = run(["git", "write-tree"])
    parent = run(["git", "rev-parse", "HEAD"])
    env = os.environ.copy()
    env["GIT_AUTHOR_NAME"] = "KevinRusev"
    env["GIT_AUTHOR_EMAIL"] = "kevinrusev1@gmail.com"
    env["GIT_AUTHOR_DATE"] = date
    env["GIT_COMMITTER_NAME"] = "KevinRusev"
    env["GIT_COMMITTER_EMAIL"] = "kevinrusev1@gmail.com"
    env["GIT_COMMITTER_DATE"] = date
    sha = run(["git", "commit-tree", tree, "-p", parent, "-m", msg], env=env)
    run(["git", "reset", "--hard", sha])
    print(f"OK: {sha[:8]} {msg}")

if __name__ == "__main__":
    commit(sys.argv[1], sys.argv[2])
