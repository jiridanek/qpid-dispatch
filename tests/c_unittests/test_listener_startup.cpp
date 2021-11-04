/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include "./qdr_doctest.hpp"
#include "./helpers.hpp"  // must come after ./qdr_doctest.hpp

#include <proton/listener.h>

#include <regex>
#include <thread>

extern "C" {
void qd_connection_manager_delete_listener(qd_dispatch_t *qd, void *impl);
}

#include <unistd.h>

void check_http_listener_startup_log_message(qd_server_config_t config, std::string listen, std::string stop)
{
    std::thread([&] {
        QDR qdr{};
        CaptureCStream css{&stderr};
        qdr.initialize("");
        css.restore();

        qd_listener_t listener{};
        qd_listener_t *li = &listener;
        li->server = qdr.qd->server;
        li->config = config;

        const size_t checkpoint = css.checkpoint();
        CHECK(qd_listener_listen(li));

        /* Websocket is opened immediately, no need to even start the worker threads */
        qd_lws_listener_close(li->http);
        qdr.deinitialize();

        free(li->http);
        qd_server_config_free(&li->config);

        std::string logging = css.str(checkpoint);
        CHECK_MESSAGE(std::regex_search(logging, std::regex{listen}), listen, " not found in ", logging);
        CHECK_MESSAGE(std::regex_search(logging, std::regex{stop}), stop, " not found in ", logging);
    }).join();
}

void check_amqp_listener_startup_log_message(qd_server_config_t config, std::string listen, std::string stop)
{
    printf("isatty %d\n", isatty(fileno(stderr)));
    QDR qdr{};
    CaptureCStream css{&stderr};
    printf("isatty %d\n", isatty(fileno(stderr)));
    fprintf(stdout, "aaaa\n");
    qdr.initialize("./minimal_trace.conf");
    css.restore();
    fprintf(stdout, "bbbb\n");

    qd_listener_t *li = qd_server_listener(qdr.qd->server);
    li->server = qdr.qd->server;
    li->config = config;

    const size_t checkpoint = css.checkpoint();
    CHECK(qd_listener_listen(li));

    {
        /* AMQP socket is opened only when proactor loop runs; meaning router has to be started */
        pn_listener_close(li->pn_listener);
        auto timer = qdr.schedule_stop(0);
        qdr.run();
    }

    qd_server_config_free(&li->config);
    free_qd_listener_t(li);

    qdr.deinitialize();

    std::string logging = css.str(checkpoint);
    CHECK_MESSAGE(std::regex_search(logging, std::regex{listen}), listen, " not found in ", logging);
    CHECK_MESSAGE(std::regex_search(logging, std::regex{stop}), stop, " not found in ", logging);
}

TEST_CASE("Start AMQP listener with zero port")
{
    std::thread([] {
        qd_server_config_t config{};
        config.port      = strdup("0");
        config.host      = strdup("localhost");
        config.host_port = strdup("localhost:0");

        check_amqp_listener_startup_log_message(config,
                                                R"EOS(SERVER \(notice\) Listening on (127.0.0.1)|(::1):(\d\d+))EOS",
                                                R"EOS(SERVER \(trace\) Listener closed on localhost:0)EOS");
    }).join();
}

//TEST_CASE("Start AMQP listener with zero port and a name")
//{
//    qd_server_config_t config{};
//    config.name      = strdup("pepa");
//    config.port      = strdup("0");
//    config.host      = strdup("localhost");
//    config.host_port = strdup("localhost:0");
//
//    check_amqp_listener_startup_log_message(
//        config,
//        R"EOS(SERVER \(notice\) Listening on (127.0.0.1)|(::1):(\d\d+) \(pepa\))EOS",
//        R"EOS(SERVER \(trace\) Listener closed on localhost:0)EOS"
//    );
//}

//TEST_CASE("Start HTTP listener with zero port")
//{
//    qd_server_config_t config{};
//    config.port      = strdup("0");
//    config.host      = strdup("localhost");
//    config.host_port = strdup("localhost:0");
//    config.http      = true;
//
//    check_http_listener_startup_log_message(
//        config,
//        R"EOS(SERVER \(notice\) Listening for HTTP on localhost:(\d\d+))EOS",
//        R"EOS(SERVER \(notice\) Stopped listening for HTTP on localhost:0)EOS"
//    );
//}
//
//TEST_CASE("Start HTTP listener with zero port and a name")
//{
//    qd_server_config_t config{};
//    config.name      = strdup("pepa");
//    config.port      = strdup("0");
//    config.host      = strdup("localhost");
//    config.host_port = strdup("localhost:0");
//    config.http      = true;
//
//    check_http_listener_startup_log_message(
//        config,
//        R"EOS(SERVER \(notice\) Listening for HTTP on localhost:(\d\d+))EOS",
//        R"EOS(SERVER \(notice\) Stopped listening for HTTP on localhost:0)EOS"
//    );
//}