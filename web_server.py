from flask import Flask, request, render_template_string
import subprocess

# Инициализация приложения
web_app = Flask(__name__)

# HTML-шаблон интерфейса
SEARCH_PAGE = '''
<!DOCTYPE html>
<html lang="ru">
<head>
    <meta charset="UTF-8">
    <title>Поиск на Stack Exchange</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            padding: 20px;
            background-color: #f9f9f9;
        }
        .container {
            max-width: 800px;
            margin: auto;
            background: white;
            padding: 20px;
            border-radius: 8px;
            box-shadow: 0 0 10px rgba(0,0,0,0.1);
        }
        h1 {
            color: #333;
        }
        form {
            margin-bottom: 20px;
        }
        input[type="text"] {
            width: 65%;
            padding: 8px;
            font-size: 1em;
            border: 1px solid #ccc;
            border-radius: 4px;
        }
        input[type="submit"] {
            padding: 8px 16px;
            font-size: 1em;
            background-color: #007bff;
            color: white;
            border: none;
            border-radius: 4px;
            cursor: pointer;
        }
        input[type="submit"]:hover {
            background-color: #0056b3;
        }
        .result-item {
            margin: 8px 0;
        }
        .pagination {
            margin-top: 20px;
        }
        .pagination a {
            margin: 0 8px;
            text-decoration: none;
            color: #007bff;
        }
        .pagination a:hover {
            text-decoration: underline;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>Поиск по Stack Exchange</h1>
        <form method="GET" action="/search">
            <input type="text" name="q" value="{{ query or '' }}" placeholder="Введите поисковый запрос" required>
            <input type="submit" value="Искать">
        </form>

        {% if results %}
            <h2>Результаты: {{ start + 1 }}–{{ end }} из {{ total_count }}</h2>
            {% for title, link in results %}
                <div class="result-item">
                    <a href="{{ link }}" target="_blank">{{ title }}</a>
                </div>
            {% endfor %}

            <div class="pagination">
                {% if has_prev %}
                    <a href="?q={{ query|urlencode }}&offset={{ prev_offset }}">&laquo; Назад</a>
                {% endif %}
                {% if has_next %}
                    <a href="?q={{ query|urlencode }}&offset={{ next_offset }}">Вперёд &raquo;</a>
                {% endif %}
            </div>

        {% elif query %}
            <p>По вашему запросу ничего не найдено.</p>
        {% endif %}
    </div>
</body>
</html>
'''

@web_app.route('/')
def index():
    return render_template_string(SEARCH_PAGE)

@web_app.route('/search')
def perform_search():
    user_query = request.args.get('q', '').strip()
    page_offset = int(request.args.get('offset', 0))

    if not user_query:
        return render_template_string(SEARCH_PAGE, query=user_query)

    parsed_results = []
    try:
        # Запуск внешней утилиты searcher с передачей поискового запроса
        process = subprocess.run(
            ['./searcher', user_query],
            capture_output=True,
            text=True,
            encoding='utf-8',
            check=True
        )
        output_lines = process.stdout.strip().split('\n')

        for line in output_lines:
            if ' | ' in line:
                parts = line.split(' | ', 1)
                if len(parts) == 2:
                    parsed_results.append((parts[0].strip(), parts[1].strip()))
    except (subprocess.CalledProcessError, OSError, ValueError):
        # При ошибке просто оставляем пустой список результатов
        pass

    total = len(parsed_results)
    page_size = 50
    start_idx = page_offset
    end_idx = min(start_idx + page_size, total)
    displayed_results = parsed_results[start_idx:end_idx]

    return render_template_string(
        SEARCH_PAGE,
        query=user_query,
        results=displayed_results,
        start=start_idx,
        end=end_idx,
        total_count=total,
        has_prev=(start_idx > 0),
        prev_offset=max(0, start_idx - page_size),
        has_next=(end_idx < total),
        next_offset=end_idx
    )

if __name__ == '__main__':
    web_app.run(host='0.0.0.0', port=5000, debug=False)
