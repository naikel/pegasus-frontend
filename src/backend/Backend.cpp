// Pegasus Frontend
// Copyright (C) 2018  Mátyás Mustoha
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.


#include "Backend.h"

#include "ScriptRunner.h"

#include <QDebug>
#include <QDir>
#include <QRegularExpression>
#include <QStandardPaths>


namespace {

QString find_writable_config_dir()
{
    const QRegularExpression replace_regex(QStringLiteral("/pegasus-frontend/pegasus-frontend$"));
    const QString dir_path = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)
        .replace(replace_regex, QStringLiteral("/pegasus-frontend"));
    if (dir_path.isEmpty()) {
        qWarning() << QObject::tr("No writable location found to save logs, file logging disabled.");
        return QString();
    }

    QDir dir(dir_path);
    if (!dir.mkpath(QLatin1String("."))) { // also true if already exists
        qWarning() << QObject::tr("Could create `%1`, file logging disabled.").arg(dir_path);
        return QString();
    }

    return dir_path;
}

QString find_writable_log_path()
{
    const QString log_path = find_writable_config_dir();
    if (log_path.isEmpty())
        return QString();

    return log_path + QLatin1String("/lastrun.log");
}

// using std::list because QTextStream is not copyable or movable,
// and neither Qt not std::vector can be used in this case
std::list<QTextStream> g_log_streams;

void on_log_message(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    // forward the message to all registered output streams
    const QByteArray preparedMsg = qFormatLogMessage(type, context, msg).toLocal8Bit();
    for (auto& stream : g_log_streams)
        stream << preparedMsg << endl;
}


void on_gamepad_config()
{
    using ScriptEvent = ScriptRunner::EventType;

    ScriptRunner::findAndRunScripts(ScriptEvent::CONFIG_CHANGED);
    ScriptRunner::findAndRunScripts(ScriptEvent::CONTROLS_CHANGED);
}

} // namespace



namespace backend {

Context::Context()
{
    setup_logging();
    setup_gamepad();
}

Context::~Context()
{
    g_log_streams.clear();
}

void Context::setup_logging()
{
    g_log_streams.emplace_back(stdout);
    qInstallMessageHandler(on_log_message);

    const QString logfile_path = find_writable_log_path();
    if (logfile_path.isEmpty())
        return;

    logfile.setFileName(logfile_path);
    if (!logfile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << QObject::tr("Could not open `%1` for writing, file logging disabled.")
                      .arg(logfile.fileName());
        return;
    }

    g_log_streams.emplace_back(&logfile);
}

void Context::setup_gamepad()
{
    padkeynav.setButtonAKey(Qt::Key_Return);
    padkeynav.setButtonBKey(Qt::Key_Escape);
    padkeynav.setButtonXKey(Qt::Key_Control);
    padkeynav.setButtonL1Key(Qt::Key_A);
    padkeynav.setButtonR1Key(Qt::Key_D);
    padkeynav.setButtonL2Key(Qt::Key_PageUp);
    padkeynav.setButtonR2Key(Qt::Key_PageDown);

    QObject::connect(QGamepadManager::instance(), &QGamepadManager::gamepadAxisEvent,
                     &padaxisnav, &GamepadAxisNavigation::onAxisEvent);

    // config change
    QObject::connect(QGamepadManager::instance(), &QGamepadManager::axisConfigured, on_gamepad_config);
    QObject::connect(QGamepadManager::instance(), &QGamepadManager::buttonConfigured, on_gamepad_config);
}

} // namespace backend
