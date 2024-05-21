# Résumé des RFC

## RFC 7230: HTTP/1.1 Message Syntax and Routing

Le RFC 7230 établit la syntaxe et le routage des messages pour HTTP/1.1, définissant les composants fondamentaux qui facilitent la communication entre les clients et les serveurs sur le Web. Il couvre les aspects essentiels tels que la structure des messages HTTP, les méthodes de requête, les codes d'état, la gestion des URI, les champs d'en-tête, le corps du message, la gestion des connexions et les considérations de sécurité.

1. **Format du message** : Les messages HTTP se composent d'une ligne de début, suivie de zéro ou plusieurs champs d'en-tête, d'une ligne vide indiquant la fin des champs d'en-tête et éventuellement d'un corps de message. La ligne de début d'un message de requête est appelée ligne de requête, tandis que la ligne de début d'un message de réponse est appelée ligne d'état.
2. **Méthodes de requête** : HTTP définit diverses méthodes de requête pour indiquer l'action souhaitée à effectuer sur la ressource identifiée. Les méthodes de requête comprennent GET, POST, PUT, DELETE, CONNECT, OPTIONS, TRACE et PATCH. Chaque méthode a un but spécifique, tel que la récupération, la création, la mise à jour ou la suppression de ressources.
3. **Codes d'état** : Les réponses HTTP incluent un code d'état qui indique le résultat de la requête. Les codes d'état sont divisés en cinq classes : 1xx (Informatif), 2xx (Réussite), 3xx (Redirection), 4xx (Erreur client) et 5xx (Erreur serveur). Les codes d'état courants incluent 200 (OK), 404 (Introuvable) et 500 (Erreur interne du serveur).
4. **URI et cible de la requête** : La cible de la requête est l'URI ou la ressource sur le serveur que le client demande. Il peut s'agir d'un chemin absolu, d'une forme d'origine, d'une forme d'autorité ou d'une forme d'astérisque. L'URI peut contenir un composant de requête pour transmettre des informations supplémentaires au serveur.
5. **Champs d'en-tête** : Les champs d'en-tête fournissent des informations supplémentaires sur la requête ou la réponse. Les champs d'en-tête de requête incluent Host, User-Agent, Accept, Content-Type et Content-Length. Les champs d'en-tête de réponse incluent Date, Server et Set-Cookie. Les champs d'en-tête peuvent être utilisés pour spécifier les informations d'identification d'authentification, les directives de contrôle de cache et le codage de contenu.
6. **Corps du message** : Le corps du message contient les données associées à la requête ou à la réponse. Il peut s'agir d'une soumission de formulaire, d'un fichier téléchargé ou du contenu d'une page Web. Le champ d'en-tête Content-Length indique la taille du corps du message, tandis que le champ d'en-tête Content-Type indique le type de support du corps du message.
7. **Gestion des connexions** : HTTP/1.1 prend en charge les connexions persistantes, qui permettent d'envoyer plusieurs requêtes et réponses sur la même connexion TCP. Le champ d'en-tête Connection peut être utilisé pour gérer ces connexions. Le mécanisme keep-alive permet au client et au serveur de spécifier une valeur de délai d'attente pour la connexion.
8. **Routage** : Les messages HTTP sont acheminés à l'aide de l'URI et de l'en-tête d'hôte HTTP. L'URI identifie la ressource, tandis que l'en-tête d'hôte identifie le serveur. Le serveur utilise la ligne de requête et l'en-tête d'hôte pour déterminer l'action appropriée à entreprendre.
9. **Considerations de sécurité** : Les personnes qui mettent en œuvre doivent être conscientes des risques de sécurité potentiels, tels que le script intersites (XSS), la falsification de requête intersites (CSRF) et l'accès non autorisé aux ressources. Le document fournit des recommandations pour atténuer ces risques, telles que l'utilisation de connexions sécurisées, la validation des données d'entrée et la mise en œuvre de mécanismes de contrôle d'accès.

## RFC 7231: HTTP/1.1 Semantics and Content

Le RFC 7231 est une spécification clé qui définit la sémantique et le contenu des messages dans HTTP/1.1, complétant ainsi les aspects de syntaxe et de routage couverts par le RFC 7230. Ce document joue un rôle crucial dans la compréhension et l'implémentation des interactions entre clients et serveurs sur le Web.

1. **Méthodes de requête** : Le RFC 7231 définit la sémantique des méthodes de requête HTTP telles que GET, POST, PUT, DELETE, HEAD, OPTIONS, CONNECT et TRACE. Chaque méthode a un but spécifique, et le serveur web doit les gérer en conséquence. Par exemple, la méthode GET est utilisée pour récupérer des ressources, tandis que la méthode POST est utilisée pour soumettre des données au serveur.
2. **En-têtes de requête et de réponse** : Le RFC explique la syntaxe et la sémantique des en-têtes HTTP utilisés dans les requêtes et les réponses. Les en-têtes fournissent des informations supplémentaires sur le message, telles que les métadonnées, le codage du contenu et le contrôle du cache. Certains en-têtes sont obligatoires, tandis que d'autres sont facultatifs ou conditionnels en fonction du contexte de la requête ou de la réponse.
3. **Codes d'état** : Le RFC 7231 fournit une liste complète des codes d'état HTTP qui indiquent le résultat d'une requête. Ces codes sont regroupés en cinq classes : informative (1xx), réussie (2xx), redirection (3xx), erreurs client (4xx) et erreurs serveur (5xx). La compréhension de ces codes d'état est cruciale pour la construction d'une mise en œuvre robuste de serveur web.
4. **Négociation de contenu** : Cette section décrit comment les serveurs et les clients peuvent négocier le format, la langue et le codage du contenu en cours de transfert. La négociation de contenu peut être soit proactive (pilotée par le serveur), soit réactive (pilotée par le client), en fonction des préférences et des capacités des deux parties.
5. **Corps du message** : Le RFC traite de la structure et du codage des corps de messages HTTP, y compris l'utilisation du codage de transfert en blocs, de la longueur du contenu et de la compression. Il aborde également la gestion du contenu partiel et des requêtes d'étendue, qui permettent aux clients de ne demander qu'une partie spécifique d'une ressource.
6. **Mise en cache** : Le RFC 7231 fournit des directives pour la mise en œuvre de mécanismes de mise en cache HTTP afin d'améliorer les performances et l'efficacité des applications web. Il explique la validité du cache, les requêtes conditionnelles et les directives de contrôle du cache qui peuvent être utilisées pour gérer les ressources mises en cache.
7. **Considérations de sécurité** : Le document souligne divers aspects de sécurité liés à HTTP/1.1, tels que l'intégrité des données, la confidentialité et l'authentification. Il recommande d'utiliser des protocoles sécurisés comme HTTPS et TLS pour protéger les informations sensibles et empêcher les accès non autorisés.

## RFC 7232: HTTP/1.1 Conditional Requests

Spécification pour les requêtes conditionnelles HTTP/1.1. Elle fournit la capacité de faire des requêtes HTTP conditionnelles en fonction de l'état actuel d'une ressource ou de l'état de la ressource au moment où elle a été modifiée pour la dernière fois. Cela aide à prévenir l'utilisation inutile du réseau et à améliorer les performances en réduisant le nombre de réponses complètes lorsque le client dispose déjà d'une version valide de la ressource.

1. **Mécanismes de requête conditionnelle** : La spécification décrit deux types de mécanismes de requête conditionnelle :
   - **Dates de dernière modification** : Ce mécanisme utilise le champ d'en-tête "Last-Modified" pour comparer la date et l'heure auxquelles le serveur d'origine pense que la ressource sélectionnée a été modifiée pour la dernière fois avec la date et l'heure fournies par le client.
   - **Balises d'entité** : Ce mécanisme utilise le champ d'en-tête "ETag", qui est un validateur opaque permettant au client de faire des requêtes conditionnelles en fonction de l'état de la ressource au moment où elle a été obtenue pour la dernière fois.
2. **Champs d'en-tête de requête** : La RFC 7232 définit plusieurs champs d'en-tête de requête qui peuvent être utilisés pour faire des requêtes conditionnelles :
   - **If-Match** : Le client peut utiliser ce champ pour rendre la requête conditionnelle à la valeur actuelle d'une balise d'entité ou d'une liste de balises d'entité.
   - **If-None-Match** : Ce champ permet au client de rendre la requête conditionnelle à la valeur actuelle d'une balise d'entité ne figurant pas dans les valeurs répertoriées.
   - **If-Modified-Since** : Ce champ rend la requête conditionnelle à la date et à l'heure auxquelles la ressource sélectionnée a été modifiée pour la dernière fois.
   - **If-Unmodified-Since** : Ce champ rend la requête conditionnelle à la ressource sélectionnée n'ayant pas été modifiée depuis la date fournie.
   - **If-Range** : Ce champ rend la requête conditionnelle à la balise d'entité ou à la date de dernière modification de la ressource sélectionnée.
3. **Codes d'état de réponse** : La spécification définit également plusieurs codes d'état HTTP qui peuvent être utilisés dans les réponses aux requêtes conditionnelles :
   - **304 (Non modifié)** : Ce code d'état indique que la ressource n'a pas été modifiée depuis la version spécifiée par les en-têtes de requête "If-Modified-Since" ou "If-None-Match".
   - **412 (Échec de la condition préalable)** : Ce code d'état indique que la condition préalable donnée dans un ou plusieurs des en-têtes de requête "If-Match", "If-None-Match", "If-Unmodified-Since", "If-Modified-Since" a été évaluée à faux.
4. **Validateurs faibles et forts** : La spécification introduit les concepts de validateurs faibles et forts. Un validateur fort est une valeur de métadonnées de représentation qui DOIT être modifiée pour garantir l'invalidation du cache chaque fois que la représentation change de manière observable par un utilisateur. Un validateur faible est une valeur de métadonnées de représentation qui PEUT changer pour diverses raisons, même lorsque les données de représentation n'ont pas changé de manière significative sur le plan sémantique.
5. **Traitement des requêtes conditionnelles** : La spécification fournit des règles détaillées sur la façon dont les serveurs doivent traiter les requêtes conditionnelles, y compris la façon de comparer les dates et les balises d'entité, de gérer plusieurs balises d'entité et de répondre lorsqu'une condition préalable échoue.
6. **Considérations de sécurité** : La spécification aborde plusieurs considérations de sécurité liées aux requêtes conditionnelles, notamment le risque de fuites d'informations et la nécessité de s'assurer que les requêtes conditionnelles ne contournent pas les contrôles d'accès.
7. **Conclusion** : La RFC 7232 fournit un cadre robuste pour rendre les requêtes HTTP conditionnelles à l'état d'une ressource, permettant une utilisation plus efficace des ressources réseau et des performances améliorées. Il s'agit d'une partie cruciale de la spécification HTTP/1.1 et est largement mis en œuvre dans les serveurs web et les clients.

## RFC 7233: HTTP/1.1 Range Requests

Les requêtes de plage HTTP/1.1, qui permettent aux clients de ne demander qu'une partie d'une ressource à un serveur. Cela peut être utile pour reprendre des téléchargements interrompus ou pour récupérer des sections spécifiques de fichiers volumineux ou de flux.

1. **Unités de plage** : La spécification définit deux types d'unités de plage : les plages d'octets et les plages de temps. Les plages d'octets sont spécifiées en termes de nombre d'octets à partir du début du fichier, tandis que les plages de temps sont spécifiées en termes de décalage de temps à partir du début du flux.
2. **En-tête de requête de plage** : Le client envoie une requête de plage à l'aide de l'en-tête de plage, qui spécifie les unités de plage et la plage à demander. Le serveur peut répondre avec un statut 206 Partial Content s'il prend en charge les requêtes de plage et peut satisfaire la demande, ou un statut 200 OK avec l'ensemble de la ressource s'il ne prend pas en charge les requêtes de plage.
3. **Plages d'octets** : Les plages d'octets sont spécifiées à l'aide du paramètre byte= dans l'en-tête de plage. Le client peut demander une seule plage (par exemple, byte=500-999) ou plusieurs plages (par exemple, byte=500-999,1000-1499). Le serveur peut répondre avec un en-tête Content-Range qui spécifie la plage d'octets du contenu retourné.
4. **Plages de temps** : Les plages de temps sont spécifiées à l'aide du paramètre clips dans l'en-tête de plage. Le client peut demander une plage de temps spécifique (par exemple, clips=0-10) ou une plage relative à l'heure actuelle (par exemple, clips=now-10). Le serveur peut répondre avec un en-tête Accept-Ranges qui spécifie les unités de plage prises en charge.
5. **Réponses multipartites** : Si le client demande plusieurs plages, le serveur peut répondre avec un type de média multipart/byteranges. Cette réponse comprend une chaîne de délimitation qui sépare les différentes parties de la réponse, et chaque partie comprend ses propres en-têtes Content-Type et Content-Range.
6. **Gestion des erreurs** : Si le client demande une plage qui n'est pas valide ou qui ne peut pas être satisfaite, le serveur peut répondre avec un statut 416 Range Not Satisfiable. Si le serveur ne prend pas en charge les requêtes de plage, il peut répondre avec un statut 200 OK et l'ensemble de la ressource.
7. **Mise en cache** : La spécification comprend des directives pour la mise en cache des réponses de plage. Si une réponse de plage est mise en cache, le cache peut stocker la réponse et l'utiliser pour satisfaire les demandes ultérieures pour la même plage.
8. **Considérations de sécurité** : La spécification comprend une discussion sur les considérations de sécurité liées aux requêtes de plage. Les serveurs doivent s'assurer que les requêtes de plage ne permettent pas aux clients d'accéder à des informations sensibles en dehors de la plage prévue.
9. **Conclusion** : Les implémentations de serveurs web doivent suivre ces directives pour prendre en charge correctement et en toute sécurité les requêtes de plage.


## RFC 7234: HTTP/1.1 Caching

Cette spécification vise à optimiser les performances des applications web par la réutilisation des réponses HTTP, en abordant les principes de mise en cache tels que la fraîcheur, la validation, et le calcul de l'âge des réponses. Elle traite également des comportements de mise en cache pour diverses méthodes de requête et codes de statut, et de l'usage de l'en-tête Cache-Control pour la gestion du cache. De plus, le RFC 7234 examine les aspects de sécurité et de stockage du cache, mettant en avant la nécessité d'une implémentation prudente pour assurer sécurité et efficacité.

1. **Terminologie** : Le RFC définit divers termes utilisés dans le contexte de la mise en cache tels que "Mis en cache", "Fraîcheur", "Validation", "Temps d'expiration explicite", "Temps d'expiration heuristique", etc. Ces termes sont cruciaux pour comprendre le mécanisme de mise en cache.
2. **Méthodes de requête** : Le document décrit le comportement de mise en cache pour différentes méthodes de requête HTTP. Par exemple, les requêtes GET sont généralement mises en cache, tandis que les requêtes POST ne le sont pas.
3. **Codes de statut de réponse** : Le comportement de mise en cache pour différents codes de statut de réponse HTTP est également défini. Par exemple, les réponses 200 (OK) sont généralement mises en cache, tandis que les réponses 201 (Créé) ne le sont pas.
4. **En-tête Cache-Control** : Il s'agit d'un en-tête clé dans la mise en cache HTTP. Il permet un contrôle direct sur le comportement de mise en cache pour les requêtes et les réponses. Le RFC explique les différentes directives telles que "public", "private", "no-cache", "no-store", "max-age", etc.
5. **Modèle de fraîcheur** : Le RFC décrit le concept de fraîcheur dans la mise en cache HTTP. Une réponse est considérée comme fraîche si son âge n'a pas dépassé sa durée de vie de fraîcheur. La durée de vie de fraîcheur peut être déterminée par l'en-tête "Cache-Control" ou l'en-tête "Expires".
6. **Modèle de validation** : Si une réponse n'est pas fraîche, elle peut être validée avant réutilisation. Le RFC explique l'utilisation des en-têtes "ETag" et "Last-Modified" pour la validation des réponses.
7. **Calcul de l'âge** : Le RFC fournit des règles pour calculer l'âge d'une réponse, qui est utilisé pour déterminer sa fraîcheur.
8. **En-tête Vary** : Cet en-tête est utilisé pour spécifier quels en-têtes de requête doivent être pris en compte lors de la décision de savoir si une réponse mise en cache peut être utilisée pour une requête ultérieure.
9. **Considérations de stockage** : Le document fournit des directives pour la gestion du stockage en cache, y compris l'expulsion des réponses mises en cache.
10. **Considérations de sécurité** : Le RFC discute des diverses considérations de sécurité liées à la mise en cache, telles que le risque d'exposition d'informations sensibles via des réponses mises en cache.
11. **Conclusion** : Le document conclut en soulignant l'importance de la mise en cache dans l'amélioration des performances et de la scalabilité des applications web, et la nécessité d'une mise en œuvre soigneuse pour assurer la correction et la sécurité.


## RFC 7235: HTTP/1.1 Authentication

RFC 7235 définit le cadre d'authentification pour HTTP/1.1, permettant une communication sécurisée entre clients et serveurs. Il introduit des mécanismes par lesquels les serveurs peuvent défier les clients pour fournir des preuves d'authentification. Ce cadre remplace les définitions d'authentification précédentes, offrant une structure plus robuste et flexible pour la gestion de l'authentification dans le protocole HTTP.

1. **Terminologie**
   - Le document utilise une terminologie spécifique liée à HTTP et à l'authentification, telle que "agent utilisateur", "serveur d'origine", "proxy", "défi", "informations d'authentification" et "domaine".
2. **Cadre d'authentification**
   - Le cadre d'authentification consiste en des messages de défi et de réponse échangés entre le client et le serveur.
   - Le serveur initie le cadre en envoyant un code d'état 401 (Non autorisé) ou 407 (Authentification proxy requise) en réponse à une demande du client.
   - Le client répond en envoyant un champ d'en-tête Authorization ou Proxy-Authorization avec la demande suivante.
3. **Registre des schémas d'authentification**
   - Le document établit le registre des schémas d'authentification HTTP, qui répertorie tous les schémas d'authentification HTTP enregistrés.
   - Le registre comprend le nom du schéma, un pointeur vers une spécification et des considérations de sécurité.
4. **Définitions des codes d'état**
   - Le document définit les codes d'état 401 (Non autorisé) et 407 (Authentification proxy requise), qui sont utilisés par le serveur pour défier le client pour l'authentification.
5. **Champs d'en-tête WWW-Authenticate et Proxy-Authenticate**
   - Ces champs d'en-tête sont utilisés par le serveur pour défier le client pour l'authentification.
   - Ils contiennent au moins un défi applicable à la ressource demandée.
   - Le défi comprend le schéma d'authentification et les paramètres.
6. **Champs d'en-tête Authorization et Proxy-Authorization**
   - Ces champs d'en-tête sont utilisés par le client pour fournir des informations d'authentification au serveur.
   - Ils contiennent les informations d'authentification du client pour le domaine de la ressource demandée.
7. **Informations d'authentification et clients inactifs**
   - Le document traite de la façon dont un client peut mettre en cache les informations d'authentification pour une réutilisation et comment gérer les clients inactifs.
   - Il recommande que les clients suppriment les informations d'authentification mises en cache après une période de temps spécifiée par l'utilisateur.
8. **Espace de protection (domaine)**
   - Le document définit un espace de protection (domaine) comme un ensemble de ressources sur le serveur qui sont protégées par un schéma d'authentification unique et/ou une base de données d'autorisation.
   - La valeur du domaine est une chaîne, généralement attribuée par le serveur, qui peut avoir une sémantique supplémentaire.
9. **Définitions des schémas d'authentification**
    - Le document fournit des définitions pour deux schémas d'authentification : Basic et Digest.
    - Le schéma Basic est un schéma simple en texte clair pour une utilisation avec le protocole HTTP.
    - Le schéma Digest est un schéma de défi-réponse utilisant un mot de passe haché pour l'authentification.
10. **Considérations relatives à l'IANA**
    - Le document détaille les procédures d'enregistrement pour les nouveaux schémas d'authentification HTTP.
    - Il répertorie également les schémas actuellement enregistrés.
11. **Considérations de sécurité**
    - Le document traite de diverses considérations de sécurité liées à l'authentification HTTP, notamment la protection des informations d'authentification, les attaques de l'homme du milieu et les attaques par déni de service.
12. **Références**
    - RFC 2616: Hypertext Transfer Protocol -- HTTP/1.1
    - RFC 2617: HTTP Authentication: Basic and Digest Access Authentication
    - RFC 7235: Hypertext Transfer Protocol (HTTP/1.1): Authentication
    - RFC 6750: The OAuth 2.0 Authorization Framework: Bearer Token Usage
    - Autres documents pertinents pour l'authentification HTTP.
