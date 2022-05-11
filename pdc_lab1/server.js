const http = require('http');

// load json
const map = require('./parts.json');

// server handler
const PORT = 1234;
function handler(req, resp) {
    const params = new URLSearchParams(req.url.replace(/^\/?\??/, ''));
    const keyword = params.get('keyword');

    let titleH1 = '', linksUl = '';
    if (keyword) {
        const results = map[keyword];
        if (!results) {
            titleH1 = `<h1>Nothing found for "${keyword}"</h1>`;
            linksUl = '';
        } else {
            titleH1 = `<h1>Search results for "${keyword}"</h1>`;
            for (const r of results) {
                const r_url = r["url"];
                const r_sum = r["sum"];
                if (r_url && r_sum) {
                    linksUl += `<li>${r_url} (score: ${r_sum})</li>`;
                }
            }
            if (linksUl) {
                linksUl = `<ul>${linksUl}</ul>`;
            }
        }
    }

    resp.writeHead(200, { 'Content-Type': 'text/html' });
    resp.end(`
        <!DOCTYPE html>
        <html lang="en">
        <head>
            <title>lab1</title>
            <script lang="text/javascript">
            function search() {
                const text = document.getElementById("text").value;
                self.location.href="http://localhost:${PORT}?keyword=" + text;
            }
            </script>
        </head>

        <body>
            <input type="text" id="text"></input>
            <button onclick="search()">Search</button>
            ${titleH1}
            ${linksUl}
        </body>
        </html>
    `);
}

// create server and listen port
const server = http.createServer((req, resp) => {
    try {
        handler(req, resp);
    } catch (ex) {
        resp.writeHead(200, { 'Content-Type': 'text/plain' });
        resp.end(`Something wrong with the server:\n\n` + ex);
    }
});
server.listen(PORT);
