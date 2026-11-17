from http.server import SimpleHTTPRequestHandler, HTTPServer
import subprocess, os

subprocess.run(["gcc", "-O2", "ludo.c", "-o", "ludo"])
if not os.path.exists("state.txt"):
    subprocess.run(["./ludo", "r"])

class LudoHandler(SimpleHTTPRequestHandler):
    def do_GET(self):
        if self.path == "/state":
            self.send_response(200)
            self.send_header("Content-Type", "text/plain")
            self.end_headers()
            with open("state.txt", "rb") as f:
                self.wfile.write(f.read())
        elif self.path == "/action/roll":
            subprocess.run(["./ludo", "d"])
            self.send_response(200); self.end_headers()
        elif self.path == "/action/reset":
            subprocess.run(["./ludo", "r"])
            self.send_response(200); self.end_headers()
        elif self.path.startswith("/action/move_"):
            token_id = self.path.split("_")[1]
            subprocess.run(["./ludo", "m", token_id])
            self.send_response(200); self.end_headers()
        else:
            super().do_GET()

    def log_message(self, format, *args):
        pass

if __name__ == "__main__":
    port = int(os.environ.get("PORT", 8080))
    print(f"Ludo Server running at http://localhost:{port}")
    HTTPServer(("0.0.0.0", port), LudoHandler).serve_forever()
