# Liste des IDs des workers qui ont démarré
started_workers = [
    1209762, 1209771, 1209764, 1209770, 1209766, 1209769, 1209767, 1209765,
    1209772, 1209768, 1209763, 1209773, 1209774, 1209775, 1209776, 1209777,
    1209778, 1209779, 1209780, 1209781
]

# Liste des IDs des workers qui ont terminé
finished_workers = [
    1209772, 1209771, 1209764, 1209777, 1209769, 1209767, 1209762, 1209765,
    1209766, 1209781, 1209773, 1209776, 1209779, 1209763, 1209774, 1209780,
    1209775, 1209778, 1209770
]
not_finished_workers = set(started_workers) - set(finished_workers)

print("Workers qui n'ont pas terminé:", not_finished_workers)
