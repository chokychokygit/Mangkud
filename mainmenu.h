#pragma once
#include "camera.h"
namespace Mangkudd {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	/// <summary>
	/// Summary for mainmenu
	/// </summary>
	public ref class mainmenu : public System::Windows::Forms::Form
	{
	public:
		mainmenu(void)
		{
			InitializeComponent();
			//
			//TODO: Add the constructor code here
			//
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~mainmenu()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::ToolStripContainer^ toolStripContainer1;
	protected:
	private: System::Windows::Forms::Button^ button1;
	private: System::Windows::Forms::Button^ button2;

	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>
		System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			System::ComponentModel::ComponentResourceManager^ resources = (gcnew System::ComponentModel::ComponentResourceManager(mainmenu::typeid));
			this->toolStripContainer1 = (gcnew System::Windows::Forms::ToolStripContainer());
			this->button2 = (gcnew System::Windows::Forms::Button());
			this->button1 = (gcnew System::Windows::Forms::Button());
			this->toolStripContainer1->ContentPanel->SuspendLayout();
			this->toolStripContainer1->SuspendLayout();
			this->SuspendLayout();
			// 
			// toolStripContainer1
			// 
			// 
			// toolStripContainer1.opencambt
			// 
			this->toolStripContainer1->ContentPanel->BackColor = System::Drawing::SystemColors::ActiveCaption;
			this->toolStripContainer1->ContentPanel->BackgroundImage = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"toolStripContainer1.opencambt.BackgroundImage")));
			this->toolStripContainer1->ContentPanel->BackgroundImageLayout = System::Windows::Forms::ImageLayout::Stretch;
			this->toolStripContainer1->ContentPanel->Controls->Add(this->button2);
			this->toolStripContainer1->ContentPanel->Controls->Add(this->button1);
			this->toolStripContainer1->ContentPanel->Size = System::Drawing::Size(999, 592);
			this->toolStripContainer1->Dock = System::Windows::Forms::DockStyle::Fill;
			this->toolStripContainer1->Location = System::Drawing::Point(0, 0);
			this->toolStripContainer1->Name = L"toolStripContainer1";
			this->toolStripContainer1->Size = System::Drawing::Size(999, 617);
			this->toolStripContainer1->TabIndex = 0;
			this->toolStripContainer1->Text = L"toolStripContainer1";
			// 
			// button2
			// 
			this->button2->Location = System::Drawing::Point(663, 515);
			this->button2->Name = L"button2";
			this->button2->Size = System::Drawing::Size(130, 66);
			this->button2->TabIndex = 1;
			this->button2->Text = L"UploadVDO";
			this->button2->UseVisualStyleBackColor = true;
			// 
			// button1
			// 
			this->button1->Location = System::Drawing::Point(208, 515);
			this->button1->Name = L"button1";
			this->button1->Size = System::Drawing::Size(175, 57);
			this->button1->TabIndex = 0;
			this->button1->Text = L"Opencamera\r\n";
			this->button1->UseVisualStyleBackColor = true;
			this->button1->Click += gcnew System::EventHandler(this, &mainmenu::button1_Click);
			// 
			// mainmenu
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(8, 16);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(999, 617);
			this->Controls->Add(this->toolStripContainer1);
			this->Name = L"mainmenu";
			this->Text = L"mainmenu";
			this->Load += gcnew System::EventHandler(this, &mainmenu::mainmenu_Load);
			this->toolStripContainer1->ContentPanel->ResumeLayout(false);
			this->toolStripContainer1->ResumeLayout(false);
			this->toolStripContainer1->PerformLayout();
			this->ResumeLayout(false);

		}
#pragma endregion
	private: System::Void mainmenu_Load(System::Object^ sender, System::EventArgs^ e) {
	}
	private: System::Void button1_Click(System::Object^ sender, System::EventArgs^ e) {
		camera^ camForm = gcnew camera();
		camForm->Show();
	}
	};
}
