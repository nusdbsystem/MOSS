# Client

Client connects to enclave KeyServer via RATLS mechanism and register his/her identity. He/She then can store encryption keys and updates access right on his/her keys. It then store models to remote storage encrypted. He/She or other clients who want to uses these model needs to have the access right and register themselves and store request input keys on KeyServer. Afterwards they can send encrypted requests to the serverless platform.

The application is developed in reference to normal client in ratls library.
