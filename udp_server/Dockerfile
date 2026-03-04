FROM python:3.12-slim

WORKDIR /app

COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

COPY app ./app

# Carpeta para persistir la DB (montada como volumen)
RUN mkdir -p /data

ENV DB_PATH=/data/messages.db
ENV UDP_HOST=0.0.0.0
ENV UDP_PORT=5005

EXPOSE 8000
EXPOSE 5005/udp

CMD ["uvicorn", "app.main:app", "--host", "0.0.0.0", "--port", "8000"]
