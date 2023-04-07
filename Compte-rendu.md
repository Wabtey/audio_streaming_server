---
authors: "Dehaye Gabriel", "Epain Florian"
---
# Compte-rendu Projet de Systèmes: streaming audio

## 24-03-2023

10-02-2023 -> 24-03-2023

Gabriel DEHAYE et
Florian EPAIN
Grp 1.2

Ce TP était instructif.

Nous avons découvert le dernier bug le dernier jour.

Nous avons eu des difficultés à la fin du TP à comprendre pourquoi notre programme ne fonctionnait pas. On ne récupérait en fait pas directement le buffer envoyé par le serveur dans le buffer que nous allions lire. On utilisait un buffer intermédiaire.

## 14-04-2023

Echo:

- géré par le programme client
  - une fois une chunk de musique reçu,
  le programme se fork pour que son enfant
  joue la musique des samples précédents plus faiblement,
  en même temps que le main joue le current.
- géré par le programme server
  - additionerai les byte du current avec les byte des précédents samples
  diminués.
