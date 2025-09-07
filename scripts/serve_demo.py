#!/usr/bin/env python3
"""
Simple HTTP server to serve the Multi-Protocol Router Simulator demo
"""

import http.server
import socketserver
import os
import sys
import webbrowser
import threading
import time

class DemoHTTPRequestHandler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=os.path.join(os.path.dirname(__file__), '..', 'docs'), **kwargs)
    
    def end_headers(self):
        # Add CORS headers for local development
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Access-Control-Allow-Methods', 'GET, POST, OPTIONS')
        self.send_header('Access-Control-Allow-Headers', 'Content-Type')
        super().end_headers()
    
    def do_GET(self):
        if self.path == '/':
            self.path = '/index.html'
        return super().do_GET()

def start_server(port=8080):
    """Start the HTTP server"""
    try:
        with socketserver.TCPServer(("", port), DemoHTTPRequestHandler) as httpd:
            print(f"🚀 Multi-Protocol Router Simulator Demo Server")
            print(f"📡 Server running at http://localhost:{port}")
            print(f"📁 Serving files from: {os.path.join(os.path.dirname(__file__), '..', 'docs')}")
            print(f"🌐 Opening browser...")
            print(f"⏹️  Press Ctrl+C to stop the server")
            print("-" * 60)
            
            # Open browser in a separate thread
            def open_browser():
                time.sleep(1)  # Wait for server to start
                webbrowser.open(f'http://localhost:{port}')
            
            browser_thread = threading.Thread(target=open_browser)
            browser_thread.daemon = True
            browser_thread.start()
            
            # Start the server
            httpd.serve_forever()
            
    except KeyboardInterrupt:
        print("\n🛑 Server stopped by user")
        sys.exit(0)
    except OSError as e:
        if e.errno == 48:  # Address already in use
            print(f"❌ Port {port} is already in use. Trying port {port + 1}...")
            start_server(port + 1)
        else:
            print(f"❌ Error starting server: {e}")
            sys.exit(1)

def main():
    """Main function"""
    port = 8080
    
    # Check if port is specified as command line argument
    if len(sys.argv) > 1:
        try:
            port = int(sys.argv[1])
        except ValueError:
            print("❌ Invalid port number. Using default port 8080.")
    
    # Check if docs directory exists
    docs_dir = os.path.join(os.path.dirname(__file__), '..', 'docs')
    if not os.path.exists(docs_dir):
        print(f"❌ Docs directory not found: {docs_dir}")
        print("Please make sure you're running this script from the project root.")
        sys.exit(1)
    
    # Check if index.html exists
    index_file = os.path.join(docs_dir, 'index.html')
    if not os.path.exists(index_file):
        print(f"❌ index.html not found: {index_file}")
        print("Please make sure the demo files are in the docs directory.")
        sys.exit(1)
    
    print("🔍 Checking demo files...")
    print(f"✅ Found docs directory: {docs_dir}")
    print(f"✅ Found index.html: {index_file}")
    print()
    
    start_server(port)

if __name__ == "__main__":
    main()
