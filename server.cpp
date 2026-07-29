#include "server.h"
#include <crow.h>
#include <thread>
#include <fstream>
#include <atomic>
#include <memory>
#include <iostream>
#include <string>
#include <filesystem>

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
				auto page = crow::mustache::load("index.html");
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