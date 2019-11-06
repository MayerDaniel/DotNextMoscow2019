#pragma once

#include <string>

template<typename T>
struct CVirtualListView {
	BEGIN_MSG_MAP(CVirtualListView)
		NOTIFY_CODE_HANDLER(LVN_COLUMNCLICK, OnColumnClick)
	ALT_MSG_MAP(1)
		REFLECTED_NOTIFY_CODE_HANDLER(LVN_COLUMNCLICK, OnColumnClick)
	END_MSG_MAP()

	struct SortInfo {
		int SortColumn = -1;
		UINT_PTR Id;
		HWND hWnd;
		bool SortAscending;
	};

protected:
	LRESULT OnColumnClick(int /*idCtrl*/, LPNMHDR hdr, BOOL& /*bHandled*/) {
		auto lv = (NMLISTVIEW*)hdr;	
		auto col = lv->iSubItem;

		auto si = FindById(hdr->idFrom);
		if (si == nullptr) {
			SortInfo s;
			s.hWnd = hdr->hwndFrom;
			s.Id = hdr->idFrom;
			m_Controls.push_back(s);
			si = m_Controls.data() + m_Controls.size() - 1;
		}

		auto oldSortColumn = si->SortColumn;
		if (col == si->SortColumn)
			si->SortAscending = !si->SortAscending;
		else {
			si->SortColumn = col;
			si->SortAscending = true;
		}

		HDITEM h;
		h.mask = HDI_FORMAT;
		CListViewCtrl list(hdr->hwndFrom);
		auto header = list.GetHeader();
		header.GetItem(si->SortColumn, &h);
		h.fmt = (h.fmt & HDF_JUSTIFYMASK) | HDF_STRING | (si->SortAscending ? HDF_SORTUP : HDF_SORTDOWN);
		header.SetItem(si->SortColumn, &h);

		if (oldSortColumn >= 0 && oldSortColumn != si->SortColumn) {
			h.mask = HDI_FORMAT;
			header.GetItem(oldSortColumn, &h);
			h.fmt = (h.fmt & HDF_JUSTIFYMASK) | HDF_STRING;
			header.SetItem(oldSortColumn, &h);
		}

		static_cast<T*>(this)->DoSort(si);
		list.RedrawItems(list.GetTopIndex(), list.GetTopIndex() + list.GetCountPerPage());

		return 0;
	}

	bool ClearSort(UINT_PTR id) {
		auto si = FindById(id);
		if (si == nullptr)
			return false;

		auto header = CListViewCtrl(si->hWnd).GetHeader();
		HDITEM h;
		h.mask = HDI_FORMAT;
		header.GetItem(si->SortColumn, &h);
		h.fmt = (h.fmt & HDF_JUSTIFYMASK) | HDF_STRING;
		header.SetItem(si->SortColumn, &h);
		si->SortColumn = -1;
		return true;
	}

	int GetSortColumn(UINT_PTR id) const {
		auto si = FindById(id);
		return si ? si->SortColumn : -1;
	}
	int IsSortAscending(UINT_PTR id) const{
		auto si = FindById(id);
		return si ? si->SortAscending : false;
	}

	const SortInfo* GetSortInfo(UINT_PTR id) const {
		return FindById(id);
	}

	static bool SortStrings(const CString& s1, const CString& s2, bool ascending) {
		return ascending ? s2.CompareNoCase(s1) > 0 : s2.CompareNoCase(s1) < 0;
	}

	static bool SortStrings(const std::string& s1, const std::string& s2, bool ascending) {
		return ascending ? s2.compare(s1) > 0 : s2.compare(s1) < 0;
	}

	template<typename Number>
	static bool SortNumbers(const Number& n1, const Number& n2, bool ascending) {
		return ascending ? n2 > n1 : n2 < n1;
	}

	void DoSort(const SortInfo*) {}

private:
	SortInfo* FindById(UINT_PTR id) {
		for (auto& info : m_Controls)
			if (info.Id == id)
				return &info;
		return nullptr;
	}
	std::vector<SortInfo> m_Controls;
};
