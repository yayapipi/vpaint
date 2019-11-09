// Copyright (C) 2012-2019 The VPaint Developers
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "FileVersionConverterDialog.h"

#include "../Global.h"

#include <QLabel>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QVBoxLayout>

FileVersionConverterDialog::FileVersionConverterDialog(
        QWidget * parent,
        const QString & fileName,
        const QString & backupName) :

    QDialog(parent)
{
    setWindowTitle(tr("File conversion required"));
    setMinimumSize(500, 200);

    // Body text
    QLabel * textLabel = new QLabel(tr(
        "The file %1 was created with an older version of VPaint and "
        "must be converted before it can be opened with your current "
        "version. Are you ok to proceed?").arg(fileName));
    textLabel->setWordWrap(true);
    textLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    // Keep old version checkbox
    QCheckBox * keepOldVersionCheckBox = new QCheckBox(
        tr("Keep a copy of the original file at %1").arg(backupName));
    keepOldVersionCheckBox->setChecked(global()->settings().keepOldVersion());
    connect(keepOldVersionCheckBox, SIGNAL(toggled(bool)), this, SLOT(keepOldVersionToggled(bool)));

    // Don't notify me next time
    QCheckBox * dontNotifyConversionCheckBox = new QCheckBox(
        tr("Don't notify me next time, just do it").arg(backupName));
    dontNotifyConversionCheckBox->setChecked(global()->settings().dontNotifyConversion());
    connect(dontNotifyConversionCheckBox, SIGNAL(toggled(bool)), this, SLOT(dontNotifyConversionToggled(bool)));

    // Dialog buttons
    QDialogButtonBox * buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    // Main layout
    QVBoxLayout * layout = new QVBoxLayout();
    layout->addWidget(textLabel);
    layout->addWidget(keepOldVersionCheckBox);
    layout->addWidget(dontNotifyConversionCheckBox);
    layout->addWidget(buttonBox);
    setLayout(layout);
}

void FileVersionConverterDialog::keepOldVersionToggled(bool checked)
{
    global()->settings().setKeepOldVersion(checked);
}

void FileVersionConverterDialog::dontNotifyConversionToggled(bool checked)
{
    global()->settings().setDontNotifyConversion(checked);
}
