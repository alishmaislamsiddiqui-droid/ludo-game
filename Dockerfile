FROM python:3.9-slim
RUN apt-get update && apt-get install -y gcc
WORKDIR /app
COPY ludo.c index.html server.py ./
EXPOSE 8080
CMD ["python3", "server.py"]
