#include "server.h"
#include <crow.h>
#include <thread>
#include <fstream>
#include <atomic>
#include <memory>
#include <iostream>
#include <string>
#include <filesystem>

static const char* default_page_html = R"html(<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1.0" />
    <title>今日待办 · 新拟态</title>
    <style>
        *,
        *::before,
        *::after {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        body {
            font-family: 'Inter', -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
            background: #e8ecf1;
            min-height: 100vh;
            display: flex;
            align-items: center;
            justify-content: center;
            padding: 24px;
            color: #2d3436;
        }
        .app {
            display: flex;
            max-width: 1200px;
            width: 100%;
            min-height: 680px;
            background: #e8ecf1;
            border-radius: 48px;
            box-shadow: 12px 12px 24px rgba(0, 0, 0, 0.08), -12px -12px 24px rgba(255, 255, 255, 0.7);
            overflow: hidden;
            backdrop-filter: blur(2px);
            transition: box-shadow 0.3s ease;
        }
        .sidebar {
            width: 280px;
            flex-shrink: 0;
            padding: 40px 28px;
            background: #e8ecf1;
            display: flex;
            flex-direction: column;
            gap: 32px;
            border-right: 2px solid rgba(255, 255, 255, 0.4);
            box-shadow: inset -4px 0 8px rgba(0, 0, 0, 0.02);
        }
        .avatar-section {
            display: flex;
            flex-direction: column;
            align-items: center;
            gap: 14px;
        }
        .avatar {
            width: 100px;
            height: 100px;
            border-radius: 50%;
            background: #e8ecf1;
            box-shadow: 8px 8px 16px rgba(0, 0, 0, 0.08), -8px -8px 16px rgba(255, 255, 255, 0.7);
            display: flex;
            align-items: center;
            justify-content: center;
            font-size: 42px;
            color: #2d3436;
            transition: box-shadow 0.2s;
            user-select: none;
        }
        .avatar:hover {
            box-shadow: 4px 4px 10px rgba(0, 0, 0, 0.06), -4px -4px 10px rgba(255, 255, 255, 0.8);
        }
        .user-name {
            font-size: 20px;
            font-weight: 600;
            letter-spacing: -0.3px;
            color: #1e272e;
        }
        .user-badge {
            font-size: 13px;
            font-weight: 400;
            color: #636e72;
            background: #e8ecf1;
            padding: 4px 16px;
            border-radius: 40px;
            box-shadow: inset 3px 3px 6px rgba(0, 0, 0, 0.04), inset -3px -3px 6px rgba(255, 255, 255, 0.6);
        }
        .nav-links {
            display: flex;
            flex-direction: column;
            gap: 10px;
            margin-top: 8px;
        }
        .nav-item {
            display: flex;
            align-items: center;
            gap: 14px;
            padding: 14px 18px;
            border-radius: 24px;
            background: #e8ecf1;
            font-size: 15px;
            font-weight: 500;
            color: #2d3436;
            cursor: default;
            transition: all 0.2s;
            box-shadow: 4px 4px 10px rgba(0, 0, 0, 0.04), -4px -4px 10px rgba(255, 255, 255, 0.5);
        }
        .nav-item .icon {
            font-size: 20px;
            opacity: 0.8;
        }
        .nav-item.active {
            box-shadow: inset 4px 4px 10px rgba(0, 0, 0, 0.06), inset -4px -4px 10px rgba(255, 255, 255, 0.7);
            color: #0984e3;
            font-weight: 600;
        }
        .nav-item:not(.active):hover {
            box-shadow: 2px 2px 8px rgba(0, 0, 0, 0.04), -2px -2px 8px rgba(255, 255, 255, 0.6);
        }
        .stats-card {
            margin-top: auto;
            padding: 22px 20px;
            border-radius: 28px;
            background: #e8ecf1;
            box-shadow: inset 4px 4px 12px rgba(0, 0, 0, 0.04), inset -4px -4px 12px rgba(255, 255, 255, 0.6);
            text-align: center;
        }
        .stats-card .number {
            font-size: 36px;
            font-weight: 700;
            color: #0984e3;
            letter-spacing: -0.5px;
        }
        .stats-card .label {
            font-size: 14px;
            color: #636e72;
            margin-top: 2px;
        }
        .progress-bar {
            margin-top: 16px;
            height: 8px;
            border-radius: 20px;
            background: #e8ecf1;
            box-shadow: inset 3px 3px 6px rgba(0, 0, 0, 0.06), inset -3px -3px 6px rgba(255, 255, 255, 0.6);
            overflow: hidden;
        }
        .progress-fill {
            height: 100%;
            width: 0%;
            border-radius: 20px;
            background: linear-gradient(135deg, #0984e3, #6c5ce7);
            box-shadow: 0 0 8px rgba(9, 132, 227, 0.3);
            transition: width 0.5s ease;
        }
        .main {
            flex: 1;
            padding: 40px 44px 40px 40px;
            display: flex;
            flex-direction: column;
            background: #e8ecf1;
        }
        .main-header {
            display: flex;
            align-items: center;
            justify-content: space-between;
            margin-bottom: 28px;
        }
        .main-header h1 {
            font-size: 28px;
            font-weight: 700;
            letter-spacing: -0.5px;
            color: #1e272e;
        }
        .main-header .date {
            font-size: 15px;
            color: #636e72;
            background: #e8ecf1;
            padding: 8px 20px;
            border-radius: 40px;
            box-shadow: inset 3px 3px 8px rgba(0, 0, 0, 0.04), inset -3px -3px 8px rgba(255, 255, 255, 0.6);
        }
        .todo-input-wrap {
            display: flex;
            gap: 12px;
            margin-bottom: 32px;
        }
        .todo-input-wrap input {
            flex: 1;
            padding: 16px 22px;
            border: none;
            border-radius: 40px;
            background: #e8ecf1;
            font-size: 15px;
            color: #1e272e;
            outline: none;
            box-shadow: inset 6px 6px 14px rgba(0, 0, 0, 0.06), inset -6px -6px 14px rgba(255, 255, 255, 0.6);
            transition: box-shadow 0.2s;
        }
        .todo-input-wrap input::placeholder {
            color: #b2bec3;
            font-weight: 400;
        }
        .todo-input-wrap input:focus {
            box-shadow: inset 4px 4px 10px rgba(0, 0, 0, 0.08), inset -4px -4px 10px rgba(255, 255, 255, 0.8);
        }
        .btn-add {
            padding: 0 32px;
            border: none;
            border-radius: 40px;
            background: #e8ecf1;
            font-size: 15px;
            font-weight: 600;
            color: #0984e3;
            cursor: pointer;
            box-shadow: 6px 6px 14px rgba(0, 0, 0, 0.06), -6px -6px 14px rgba(255, 255, 255, 0.6);
            transition: all 0.2s;
            white-space: nowrap;
        }
        .btn-add:hover {
            box-shadow: 4px 4px 10px rgba(0, 0, 0, 0.05), -4px -4px 10px rgba(255, 255, 255, 0.7);
            transform: scale(1.01);
        }
        .btn-add:active {
            box-shadow: inset 4px 4px 10px rgba(0, 0, 0, 0.06), inset -4px -4px 10px rgba(255, 255, 255, 0.6);
            transform: scale(0.98);
        }
        .todo-list {
            flex: 1;
            display: flex;
            flex-direction: column;
            gap: 10px;
            overflow-y: auto;
            padding-right: 4px;
            max-height: 420px;
        }
        .todo-list::-webkit-scrollbar {
            width: 5px;
        }
        .todo-list::-webkit-scrollbar-track {
            background: transparent;
        }
        .todo-list::-webkit-scrollbar-thumb {
            background: #d1d8e0;
            border-radius: 20px;
            box-shadow: inset 0 0 4px rgba(0, 0, 0, 0.04);
        }
        .todo-item {
            display: flex;
            align-items: center;
            gap: 14px;
            padding: 16px 22px;
            border-radius: 28px;
            background: #e8ecf1;
            box-shadow: 4px 4px 12px rgba(0, 0, 0, 0.04), -4px -4px 12px rgba(255, 255, 255, 0.5);
            transition: all 0.25s ease;
            animation: fadeInUp 0.3s ease forwards;
            opacity: 0;
        }
        .todo-item:hover {
            box-shadow: 2px 2px 8px rgba(0, 0, 0, 0.04), -2px -2px 8px rgba(255, 255, 255, 0.6);
        }
        .todo-item.completed {
            opacity: 0.65;
        }
        .todo-item.completed .todo-text {
            text-decoration: line-through;
            color: #b2bec3;
        }
        .todo-check {
            width: 26px;
            height: 26px;
            flex-shrink: 0;
            border-radius: 50%;
            background: #e8ecf1;
            box-shadow: inset 3px 3px 8px rgba(0, 0, 0, 0.06), inset -3px -3px 8px rgba(255, 255, 255, 0.6);
            cursor: pointer;
            display: flex;
            align-items: center;
            justify-content: center;
            transition: all 0.2s;
            font-size: 14px;
            color: transparent;
            user-select: none;
        }
        .todo-check.checked {
            box-shadow: 3px 3px 8px rgba(0, 0, 0, 0.04), -3px -3px 8px rgba(255, 255, 255, 0.5);
            background: #0984e3;
            color: white;
        }
        .todo-check.checked::after {
            content: "✓";
            font-weight: 700;
            font-size: 16px;
        }
        .todo-text {
            flex: 1;
            font-size: 15px;
            font-weight: 450;
            color: #1e272e;
            word-break: break-word;
            transition: color 0.2s;
        }
        .todo-delete {
            width: 32px;
            height: 32px;
            border-radius: 50%;
            border: none;
            background: #e8ecf1;
            font-size: 16px;
            color: #e17055;
            cursor: pointer;
            box-shadow: 4px 4px 10px rgba(0, 0, 0, 0.04), -4px -4px 10px rgba(255, 255, 255, 0.5);
            display: flex;
            align-items: center;
            justify-content: center;
            transition: all 0.2s;
            flex-shrink: 0;
            opacity: 0.5;
        }
        .todo-delete:hover {
            opacity: 1;
            box-shadow: 2px 2px 8px rgba(0, 0, 0, 0.04), -2px -2px 8px rgba(255, 255, 255, 0.6);
            transform: scale(1.05);
        }
        .todo-delete:active {
            box-shadow: inset 3px 3px 8px rgba(0, 0, 0, 0.06), inset -3px -3px 8px rgba(255, 255, 255, 0.6);
            transform: scale(0.92);
        }
        .empty-state {
            text-align: center;
            padding: 48px 20px;
            color: #b2bec3;
            font-size: 15px;
            border-radius: 32px;
            background: #e8ecf1;
            box-shadow: inset 4px 4px 14px rgba(0, 0, 0, 0.03), inset -4px -4px 14px rgba(255, 255, 255, 0.4);
        }
        .empty-state .big-icon {
            font-size: 48px;
            display: block;
            margin-bottom: 12px;
            opacity: 0.5;
        }
        @keyframes fadeInUp {
            0% { opacity: 0; transform: translateY(12px) scale(0.98); }
            100% { opacity: 1; transform: translateY(0) scale(1); }
        }
        @media (max-width: 820px) {
            .app { flex-direction: column; border-radius: 32px; min-height: auto; }
            .sidebar { width: 100%; border-right: none; border-bottom: 2px solid rgba(255,255,255,0.3); padding: 28px 24px; flex-direction: row; flex-wrap: wrap; align-items: center; gap: 16px 24px; box-shadow: inset 0 -4px 8px rgba(0,0,0,0.02); }
            .avatar-section { flex-direction: row; gap: 16px; }
            .avatar { width: 64px; height: 64px; font-size: 28px; }
            .user-name { font-size: 17px; }
            .nav-links { flex-direction: row; flex-wrap: wrap; gap: 6px; margin-top: 0; }
            .nav-item { padding: 10px 16px; font-size: 13px; gap: 8px; }
            .nav-item .icon { font-size: 16px; }
            .stats-card { margin-top: 0; padding: 14px 20px; min-width: 140px; }
            .stats-card .number { font-size: 28px; }
            .main { padding: 28px 24px; }
            .main-header { flex-wrap: wrap; gap: 12px; }
            .main-header h1 { font-size: 22px; }
        }
        @media (max-width: 480px) {
            body { padding: 12px; }
            .app { border-radius: 24px; }
            .sidebar { padding: 20px 16px; gap: 12px; }
            .avatar { width: 50px; height: 50px; font-size: 22px; }
            .nav-item { padding: 8px 12px; font-size: 12px; }
            .main { padding: 20px 16px; }
            .todo-input-wrap { flex-direction: column; }
            .btn-add { padding: 14px; width: 100%; justify-content: center; }
            .todo-item { padding: 12px 16px; gap: 10px; }
            .todo-text { font-size: 14px; }
            .main-header .date { font-size: 12px; padding: 4px 14px; }
        }
    </style>
</head>
<body>
    <div class="app">
        <aside class="sidebar">
            <div class="avatar-section">
                <div class="avatar">&#x1F9D1;&#x200D;&#x1F4BB;</div>
                <div>
                    <div class="user-name">林 一</div>
                    <div class="user-badge">今日专注</div>
                </div>
            </div>
            <nav class="nav-links">
                <div class="nav-item active"><span class="icon">&#x1F4CB;</span> 今日待办</div>
                <div class="nav-item"><span class="icon">&#x1F4C5;</span> 日程</div>
                <div class="nav-item"><span class="icon">&#x1F3F7;&#xFE0F;</span> 标签</div>
                <div class="nav-item"><span class="icon">&#x2699;&#xFE0F;</span> 设置</div>
            </nav>
            <div class="stats-card">
                <div class="number" id="progressPercent">0%</div>
                <div class="label">完成进度</div>
                <div class="progress-bar">
                    <div class="progress-fill" id="progressFill" style="width:0%;"></div>
                </div>
            </div>
        </aside>
        <main class="main">
            <div class="main-header">
                <h1>&#x1F4CC; 今日待办</h1>
                <span class="date" id="todayDate"></span>
            </div>
            <div class="todo-input-wrap">
                <input type="text" id="todoInput" placeholder="输入新任务，按 Enter 或点击添加..." autocomplete="off" />
                <button class="btn-add" id="addBtn">+ 添加</button>
            </div>
            <div class="todo-list" id="todoList">
                <div class="empty-state">
                    <span class="big-icon">&#x2728;</span>
                    暂无待办，添加一条吧！
                </div>
            </div>
        </main>
    </div>
    <script>
        (function() {
        'use strict';
        const todoListEl = document.getElementById('todoList');
        const todoInput = document.getElementById('todoInput');
        const addBtn = document.getElementById('addBtn');
        const progressFill = document.getElementById('progressFill');
        const progressPercent = document.getElementById('progressPercent');
        let todos = [];
        const initialTodos = [
        { id: 1, text: '撰写项目周报', completed: false },
        { id: 2, text: '设计系统 review', completed: true },
        { id: 3, text: '团队同步会议', completed: false },
        { id: 4, text: '整理设计稿归档', completed: false },
        ];
        function generateId() {
        return Date.now() + Math.floor(Math.random() * 1000);
        }
        function formatDate() {
        const d = new Date();
        const y = d.getFullYear();
        const m = String(d.getMonth() + 1).padStart(2, '0');
        const day = String(d.getDate()).padStart(2, '0');
        const weekdays = ['日', '一', '二', '三', '四', '五', '六'];
        const w = weekdays[d.getDay()];
        return y + '年' + m + '月' + day + '日 · 周' + w;
        }
        function render() {
        document.getElementById('todayDate').textContent = formatDate();
        todoListEl.innerHTML = '';
        if (todos.length === 0) {
        todoListEl.innerHTML = '<div class="empty-state"><span class="big-icon">&#x1F3AF;</span>暂无待办，添加一条吧！</div>';
        updateProgress();
        return;
        }
        const sorted = [...todos].sort((a, b) => {
        if (a.completed === b.completed) return 0;
        return a.completed ? 1 : -1;
        });
        sorted.forEach((todo, index) => {
        const item = document.createElement('div');
        item.className = 'todo-item' + (todo.completed ? ' completed' : '');
        item.style.animationDelay = Math.min(index * 40, 300) + 'ms';
        const check = document.createElement('div');
        check.className = 'todo-check' + (todo.completed ? ' checked' : '');
        check.setAttribute('role', 'button');
        check.setAttribute('aria-label', '切换完成状态');
        check.dataset.id = todo.id;
        const text = document.createElement('span');
        text.className = 'todo-text';
        text.textContent = todo.text;
        const del = document.createElement('button');
        del.className = 'todo-delete';
        del.setAttribute('aria-label', '删除待办');
        del.innerHTML = '&#x2715;';
        del.dataset.id = todo.id;
        item.appendChild(check);
        item.appendChild(text);
        item.appendChild(del);
        todoListEl.appendChild(item);
        });
        updateProgress();
        }
        function updateProgress() {
        const total = todos.length;
        const done = todos.filter(t => t.completed).length;
        const pct = total === 0 ? 0 : Math.round((done / total) * 100);
        progressFill.style.width = pct + '%';
        progressPercent.textContent = pct + '%';
        }
        function addTodo(text) {
        const trimmed = text.trim();
        if (!trimmed) return false;
        todos.push({ id: generateId(), text: trimmed, completed: false });
        render();
        return true;
        }
        function toggleTodo(id) {
        const todo = todos.find(t => t.id === id);
        if (!todo) return;
        todo.completed = !todo.completed;
        render();
        }
        function deleteTodo(id) {
        todos = todos.filter(t => t.id !== id);
        render();
        }
        todoListEl.addEventListener('click', (e) => {
        const target = e.target;
        const check = target.closest('.todo-check');
        if (check) { const id = Number(check.dataset.id); if (!isNaN(id)) toggleTodo(id); return; }
        const del = target.closest('.todo-delete');
        if (del) { const id = Number(del.dataset.id); if (!isNaN(id)) deleteTodo(id); return; }
        });
        function handleAdd() {
        const val = todoInput.value;
        const success = addTodo(val);
        if (success) todoInput.value = '';
        todoInput.focus();
        }
        addBtn.addEventListener('click', handleAdd);
        todoInput.addEventListener('keydown', (e) => {
        if (e.key === 'Enter') { e.preventDefault(); handleAdd(); }
        });
        todos = JSON.parse(JSON.stringify(initialTodos));
        render();
        todoInput.focus();
        })();
    </script>
</body>
</html>)html";

static std::unique_ptr<crow::SimpleApp> app;
static std::thread web_thread;
static std::atomic<bool> server_running{ false };

int serverStart(const char* addr, short port, const char* dir, const char* file, bool safe) {
	if (server_running) {
		return 1;
	}
	app = std::make_unique<crow::SimpleApp>();
	auto& a = *app;
	web_thread = std::thread([&a, addr = std::string(addr), port, dir = std::string(dir), file = std::string(file), safe]() {
		if (!file.empty()) {
			CROW_ROUTE(a, "/")([file]() {
				std::ifstream ifs(std::filesystem::u8path(file), std::ios::binary);
				if (!ifs) return crow::response(404);
				std::string content((std::istreambuf_iterator<char>(ifs)), {});
				crow::response res(content);
				std::size_t dot = file.find_last_of('.');
				if (dot != std::string::npos)
					res.add_header("Content-Type", crow::response::get_mime_type(file.substr(dot + 1)));
				return res;
			});
			if (!safe) {
				auto parent = std::filesystem::u8path(file).parent_path().string();
				CROW_ROUTE(a, "/<path>")([parent](const std::string& path) {
					std::ifstream ifs(std::filesystem::u8path(parent) / path, std::ios::binary);
					if (!ifs) return crow::response(404);
					std::string content((std::istreambuf_iterator<char>(ifs)), {});
					crow::response res(content);
					std::size_t dot = path.find_last_of('.');
					if (dot != std::string::npos)
						res.add_header("Content-Type", crow::response::get_mime_type(path.substr(dot + 1)));
					return res;
				});
			}
		} else if (!dir.empty()) {
			CROW_ROUTE(a, "/")([dir]() {
				std::ifstream file(std::filesystem::u8path(dir) / "index.html", std::ios::binary);
				if (!file) return crow::response(404);
				std::string content((std::istreambuf_iterator<char>(file)), {});
				crow::response res(content);
				res.add_header("Content-Type", "text/html");
				return res;
				});
			CROW_ROUTE(a, "/<path>")([dir](const std::string& path) {
				std::ifstream file(std::filesystem::u8path(dir) / path, std::ios::binary);
				if (!file) return crow::response(404);
				std::string content((std::istreambuf_iterator<char>(file)), {});
				crow::response res(content);
				std::size_t dot = path.find_last_of('.');
				if (dot != std::string::npos)
					res.add_header("Content-Type", crow::response::get_mime_type(path.substr(dot + 1)));
				return res;
				});
		} else {
			CROW_ROUTE((*app), "/")([]() {
				static auto page = crow::mustache::compile(default_page_html);
				return page.render();
				});
		}
		a.bindaddr(addr).port(port).multithreaded().run();
		server_running.store(false);
		});
	server_running.store(true);
	return 0;
}

int serverStop() {
	if (!server_running)
		return 1;
	app->stop();
	if (web_thread.joinable())
		web_thread.join();
	server_running.store(false);
	return 0;
}

bool serverRunning() {
	return (bool)server_running;
}