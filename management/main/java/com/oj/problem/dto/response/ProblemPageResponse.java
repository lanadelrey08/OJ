package com.oj.problem.dto.response;

import java.util.List;

public class ProblemPageResponse {

    private long total;
    private int page;
    private int size;
    private List<ProblemListItemResponse> problems;

    public long getTotal() {
        return total;
    }

    public void setTotal(long total) {
        this.total = total;
    }

    public int getPage() {
        return page;
    }

    public void setPage(int page) {
        this.page = page;
    }

    public int getSize() {
        return size;
    }

    public void setSize(int size) {
        this.size = size;
    }

    public List<ProblemListItemResponse> getProblems() {
        return problems;
    }

    public void setProblems(List<ProblemListItemResponse> problems) {
        this.problems = problems;
    }
}
