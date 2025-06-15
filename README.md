# wator
WebAssembly port of the wator visualization from xlock (that itself is based on Dewdney's Wa-Tor, water torus simulation)

[Live demo](https://gregstoll.com/wator/)
[Project writeup](https://gregstoll.wordpress.com/2025/06/14/wa-tor-simulation-watch-the-sharks-eat-the-fish/)

Based on xlockmore-5.83
- Primary site: https://www.sillycycle.com/xlock
- Alternate site: http://ibiblio.org/pub/Linux/X11/screensavers
- Web page: https://www.sillycycle.com/xlockmore.html
- Maintainer: David A. Bagley <bagleyd AT verizon.net>
- Primary key fingerprint: 1B79 C847 8F23 6DCD ADC0  D6AC CF15 9707 3779 19AE

To build:
- [Set up the Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html)
- Run `emcc -O3 wator.cpp -s EXPORTED_RUNTIME_METHODS=HEAPU8 -o water.html`
  - (but ignore `wator.html`, instead use `wator_simulation.html`)
- To emit source maps, run `emcc wator.cpp -s EXPORTED_RUNTIME_METHODS=HEAPU8 -o wator.html -g -gsource-map`

To test locally:
- Run `python -m http.server`
- In a web browser, open 127.0.0.1:8000