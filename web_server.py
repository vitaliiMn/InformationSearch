from flask import Flask, request, render_template_string
import subprocess
import sys

app = Flask(__name__)

HOME = """
<!DOCTYPE html>
<html>
<head>
    <meta charset="utf-8">
    <title>Поиск</title>
    <style>
        body { font-family: sans-serif; margin: 20px; }
        form { margin-bottom: 20px; }
        input[type=text] { width: 60%; padding: 8px; }
        input[type=submit] { padding: 8px 16px; }
        .res { margin: 10px 0; }
        .pager { margin-top: 15px; }
    </style>
</head>
<body>
    <h1>Поиск по Stack Exchange</h1>
    <form method="GET" action="/search">
        <input type="text" name="q" value="{{ query|default('', true) }}" required>
        <input type="submit" value="Найти">
    </form>
    {% if results %}
        <h2>Результаты ({{ start+1 }}–{{ end }} из {{ total }})</h2>
        {% for title, url in results %}
            <div class="res"><a href="{{ url }}">{{ title }}</a></div>
        {% endfor %}
        <div class="pager">
            {% if start > 0 %}<a href="?q={{ query|urlencode }}&offset={{ start-50 }}">← Назад</a> | {% endif %}
            {% if end < total %}<a href="?q={{ query|urlencode }}&offset={{ end }}">Вперёд →</a>{% endif %}
        </div>
    {% elif query %}
        <p>Ничего не найдено.</p>
    {% endif %}
</body>
</html>
"""

@app.route('/')
def home():
    return render_template_string(HOME)

@app.route('/search')
def search():
    query = request.args.get('q', '').strip()
    offset = int(request.args.get('offset', 0))
    if not query:
        return render_template_string(HOME, query=query)

    results = []
    try:
        out = subprocess.run(["./searcher", query], capture_output=True, text=True, encoding='utf-8', check=True).stdout
        for line in out.splitlines():
            if " | " in line:
                parts = line.split(" | ", 1)
                if len(parts) == 2:
                    results.append((parts[0], parts[1]))
    except Exception:
        pass

    start = offset
    end = min(start + 50, len(results))
    paginated = results[start:end]
    return render_template_string(HOME,
                                 query=query,
                                 results=paginated,
                                 start=start,
                                 end=end,
                                 total=len(results))

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
