import http.client
import time

def test_keep_alive(host: str, port: int = 8080, path: str = "/") -> None:
    """
    Tests the HTTP Keep-Alive feature by sending two sequential requests
    over a single TCP connection.

    Args:
        host: The hostname or IP address of the web server (e.g., 'www.google.com').
        port: The port number (default is 80 for HTTP).
        path: The path to request (default is '/').
    """
    print(f"Connecting to **{host}:{port}**...")
    
    # 1. Establish the connection
    try:
        # Use HTTPConnection for a standard HTTP connection
        conn = http.client.HTTPConnection(host, port, timeout=5)
        print("Connection established.")
    except Exception as e:
        print(f"ERROR: Could not establish connection. {e}")
        return

    # --- Request 1 ---
    try:
        print("\n--- Sending Request 1 ---")
        # Explicitly request Keep-Alive (though it's often the default in HTTP/1.1)
        headers = {"Connection": "keep-alive", "User-Agent": "KeepAliveTester/1.0"}
        conn.request("GET", path, headers=headers)
        
        # Get the response
        response = conn.getresponse()
        
        print(f"Status: **{response.status} {response.reason}**")
        
        # Check if the server's response header allows Keep-Alive
        connection_header = response.getheader("Connection", "close").lower()
        if "keep-alive" in connection_header:
            print(f"Server sent 'Connection: **{connection_header}**'. Connection is likely kept open.")
        else:
            print(f"Server sent 'Connection: **{connection_header}**'. Connection will likely close.")
            
        # Read the body to finish Request 1 properly
        _ = response.read() 

    except Exception as e:
        print(f"ERROR during Request 1: {e}")
        conn.close()
        return
    
    # Pause for a moment to simulate real-world delay
    time.sleep(6)

    # --- Request 2 ---
    # Crucially, we use the *same* 'conn' object without re-establishing it.
    try:
        print("\n--- Sending Request 2 (reusing the same connection) ---")
        
        # The 'Connection: keep-alive' header is often not needed for subsequent requests
        # on the *client* side as long as the same connection object is used.
        conn.request("GET", path, headers={"User-Agent": "KeepAliveTester/1.0-Reuse"})
        
        # Get the second response
        response = conn.getresponse()
        
        print(f"Status: **{response.status} {response.reason}**")
        
        # Check the header again
        connection_header = response.getheader("Connection", "close").lower()
        print(f"Server sent 'Connection: **{connection_header}**'.")
        
        # Read the body to finish Request 2
        _ = response.read()

        print("\nSuccessfully sent two requests on the same connection.")
        print("To verify, you would typically monitor network traffic (e.g., with Wireshark)")
        print("and see a single TCP handshake for both requests.")

    except http.client.NotConnected:
        # This exception indicates the server explicitly closed the connection 
        # after the first request, meaning Keep-Alive failed or was disallowed.
        print("\n**Keep-Alive FAILED**.")
        print("The server closed the connection after the first request.")
    except Exception as e:
        print(f"ERROR during Request 2: {e}")
    finally:
        # 3. Close the connection
        conn.close()
        print("\nConnection closed by client.")

# --- Execution ---
if __name__ == "__main__":
    # NOTE: You should use a known HTTP server that supports Keep-Alive.
    # Replace 'example.com' with the server you want to test.
    # Use a well-known site or your own local server (e.g., 'localhost:8000')
    
    # Test 1: Known site (often redirects or uses HTTPS, which needs http.client.HTTPSConnection)
    # Using a general example, but you should ideally target a plain HTTP server (port 80)
    # or a local testing server for controlled results.
    try:
        test_host = "localhost" # A good site for testing requests
        test_keep_alive(host=test_host, path="/")
    except NameError:
        print("\nERROR: Please ensure the 'httpbin.org' is accessible or change the 'test_host' variable.")