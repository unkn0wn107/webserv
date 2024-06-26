FROM nginx

COPY entrypoint.sh /docker-entrypoint.sh
RUN chmod +x /docker-entrypoint.sh

HEALTHCHECK --interval=1s --timeout=60s \
  CMD [ -f /tmp/healthy ] || (curl -f http://localhost:8080/ && touch /tmp/healthy || exit 1)

  ENTRYPOINT ["/docker-entrypoint.sh"]
