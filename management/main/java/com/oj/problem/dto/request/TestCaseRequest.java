package com.oj.problem.dto.request;

import javax.validation.constraints.Min;
import javax.validation.constraints.NotBlank;
import javax.validation.constraints.NotNull;

public class TestCaseRequest {

    @NotBlank(message = "测试输入不能为空")
    private String input;

    @NotBlank(message = "测试输出不能为空")
    private String output;

    @NotNull(message = "是否样例不能为空")
    private Boolean isSample;

    @Min(value = 0, message = "分值不能小于0")
    private Integer score = 0;

    public String getInput() {
        return input;
    }

    public void setInput(String input) {
        this.input = input;
    }

    public String getOutput() {
        return output;
    }

    public void setOutput(String output) {
        this.output = output;
    }

    public Boolean getIsSample() {
        return isSample;
    }

    public void setIsSample(Boolean isSample) {
        this.isSample = isSample;
    }

    public Integer getScore() {
        return score;
    }

    public void setScore(Integer score) {
        this.score = score;
    }
}
