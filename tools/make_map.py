#!/usr/bin/env python3
"""Erzeugt kompakte Offline-Karten (gzip) fuer die Weboberflaeche aus den
DisasterMesh-GeoJSON-Paketen (OpenStreetMap, ODbL 1.0).
Format: {"name","attr","bbox","view","l":[[art, x0,y0,dx,dy,...]],"p":[[art,name,x,y]]}
Koordinaten als ganze Zahlen in 1e-5 Grad, Linien delta-kodiert."""
import json, gzip, sys, os
SRC = sys.argv[1] if len(sys.argv) > 1 else '/workspace/DisasterMesh/maps'
OUT = sys.argv[2] if len(sys.argv) > 2 else os.path.join(os.path.dirname(__file__), '..', 'web', 'static', 'map')
MAJOR = {'motorway','trunk','primary','secondary','tertiary','motorway_link','trunk_link','primary_link','secondary_link','tertiary_link'}
MINOR = {'residential','unclassified','living_street','service','road','pedestrian'}
PATH = {'track','footway','path','cycleway','steps','bridleway'}
POI = {'pharmacy':'apo','hospital':'med','doctors':'med','clinic':'med','fire_station':'fw','police':'pol',
       'drinking_water':'h2o','water_point':'h2o','shelter':'sh','toilets':'wc','townhall':'amt','school':'sch'}
SHOP = {'supermarket','convenience','bakery','butcher','greengrocer'}

def q(v): return int(round(v * 1e5))

def kind(pr, geom):
    if geom in ('LineString','Polygon'):
        if 'building' in pr: return 'b'
        h = pr.get('highway')
        if h in MAJOR: return 'h'
        if h in MINOR: return 'r'
        if h in PATH: return 'f'
        if pr.get('railway'): return 'rw'
        w = pr.get('waterway')
        if w == 'river' or pr.get('name') == 'Ahr': return 'w'
        if w: return 's'
        if pr.get('natural') == 'water' or pr.get('water'): return 'wa'
        if pr.get('landuse') == 'vineyard': return 'v'
        if pr.get('landuse') in ('forest',) or pr.get('natural') == 'wood': return 'wd'
        return None
    if 'place' in pr and pr.get('name'): return 'pl:' + pr['place']
    a = pr.get('amenity') or pr.get('healthcare')
    if a in POI: return POI[a]
    if pr.get('shop') in SHOP: return 'shop'
    return None

def conv(fn, name, keep_buildings):
    d = json.load(open(os.path.join(SRC, fn)))
    L, P = [], []
    for f in d['features']:
        g, pr = f['geometry'], f['properties']
        k = kind(pr, g['type'])
        if not k or (k == 'b' and not keep_buildings): continue
        if g['type'] == 'Point':
            x, y = g['coordinates']
            P.append([k, pr.get('name', ''), q(x), q(y)])
            continue
        cs = g['coordinates'][0] if g['type'] == 'Polygon' else g['coordinates']
        if len(cs) < 2: continue
        row, px, py = [k], 0, 0
        for i, (x, y) in enumerate(cs):
            X, Y = q(x), q(y)
            if i and X == px and Y == py: continue
            row += [X, Y] if i == 0 else [X - px, Y - py]
            px, py = X, Y
        L.append(row)
    out = {'name': name, 'attr': d.get('attribution', '© OpenStreetMap contributors · ODbL 1.0'),
           'bbox': d.get('bbox'), 'view': d.get('viewBounds') or d.get('bbox'), 'l': L, 'p': P}
    if not out['bbox']:  # GeoJSON ohne bbox: Grenzen aus Orten/POIs und Linien ohne Gewässer (Ahr/Bäche reichen weit über den Ausschnitt hinaus)
        xs, ys = [p[2] for p in P], [p[3] for p in P]
        for l in L:
            if l[0] in ('w', 's', 'wa'): continue
            x, y = l[1], l[2]; xs.append(x); ys.append(y)
            for i in range(3, len(l) - 1, 2): x += l[i]; y += l[i + 1]; xs.append(x); ys.append(y)
        out['bbox'] = out['view'] = [round(min(xs) / 1e5, 5), round(min(ys) / 1e5, 5), round(max(xs) / 1e5, 5), round(max(ys) / 1e5, 5)]
    raw = json.dumps(out, ensure_ascii=False, separators=(',', ':')).encode()
    os.makedirs(OUT, exist_ok=True)
    base = os.path.join(OUT, fn.replace('.geojson', '.json'))
    with gzip.open(base + '.gz', 'wb', 9) as z: z.write(raw)
    print(f'{fn}: {len(L)} Linien, {len(P)} Punkte, {len(raw)//1024} KB roh, {os.path.getsize(base + ".gz")//1024} KB gz')

conv('ahrtal.geojson', 'Ahrtal (Übersicht)', False)
conv('heimersheim.geojson', 'Heimersheim (Detail mit Gebäuden)', True)
conv('dernau.geojson', 'Dernau · Walporzheim · Ahrweiler', False)
conv('altenahr.geojson', 'Altenahr · Mayschoß · Rech', False)
conv('sinzig.geojson', 'Sinzig · Westum · Koisdorf', False)
