#include "apk/projectitemsmodel.h"
#include "base/application.h"

ProjectItemsModel::~ProjectItemsModel()
{
    qDeleteAll(projects);
}

Project *ProjectItemsModel::open(const QString &filename, bool unpack)
{
    auto project = new Project(filename);
    if (unpack) {
        project->unpack();
    }

    beginInsertRows(QModelIndex(), rowCount(), rowCount());
        projects.append(project);
    endInsertRows();
    emit added(project);

    connect(project, &Project::changed, [=]() {
        const int row = indexOf(project);
        emit dataChanged(index(row, 0), index(row, ColumnCount - 1));
        emit changed(project);
    });

    return project;
}

bool ProjectItemsModel::close(Project *project)
{
    const int row = indexOf(project);
    if (row == -1) {
        return false;
    }

    beginRemoveRows(QModelIndex(), row, row);
        projects.removeAt(row);
        delete project;
    endRemoveRows();

    emit removed(project);
    return true;
}

Project *ProjectItemsModel::existing(const QString &filename) const
{
    QListIterator<Project *> it(projects);
    while (it.hasNext()) {
        Project *iteration = it.next();
        if (iteration->getOriginalPath() == QDir::toNativeSeparators(filename)) {
            return iteration;
        }
    }
    return nullptr;
}

int ProjectItemsModel::indexOf(Project *project) const
{
    return projects.indexOf(project);
}

QVariant ProjectItemsModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid()) {
        const int column = index.column();
        const Project *project = static_cast<Project *>(index.internalPointer());
        if (role == Qt::DisplayRole) {
            switch (column) {
            case TitleColumn:
                return project->getTitle();
            case OriginalPathColumn:
                return project->getOriginalPath();
            case ContentsPathColumn:
                return project->getContentsPath();
            case IsUnpackedColumn:
                return project->getState().isUnpacked();
            case IsModifiedColumn:
                return project->getState().isModified();
            }
        } else if (role == Qt::DecorationRole) {
            switch (column) {
            case TitleColumn:
                return project->getThumbnail();
            case StatusColumn:
                switch (project->getState().getCurrentStatus()) {
                case ProjectState::Status::Normal:
                    return app->icons.get("state-idle.png");
                case ProjectState::Status::Unpacking:
                    return app->icons.get("state-open.png");
                case ProjectState::Status::Packing:
                case ProjectState::Status::Signing:
                case ProjectState::Status::Optimizing:
                    return app->icons.get("state-save.png");
                case ProjectState::Status::Installing:
                    return app->icons.get("state-install.png");
                case ProjectState::Status::Errored:
                    return app->icons.get("state-error.png");
                }
            }
        }
    }
    return QVariant();
}

QModelIndex ProjectItemsModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    if (row >= 0 && row < projects.count()) {
        return createIndex(row, column, projects.at(row));
    }
    return QModelIndex();
}

int ProjectItemsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return projects.count();
}
